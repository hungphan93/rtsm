#include "system_info_reader_linux.hpp"
#include <charconv>
#include <fstream>
#include <regex>
#include <thread>

namespace adapter {
namespace linux2 {

namespace {

std::string extract_value(const std::string& line) {
    auto pos = line.find(':');
    return (pos == std::string::npos || pos + 2 >= line.size())
               ? "" : line.substr(pos + 2);
}

template<typename T>
bool parse_number(const std::string& text, T& out) {
    auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), out);
    return ec == std::errc();
}

struct cpu_times {
    uint64_t user = 0;
    uint64_t nice = 0;
    uint64_t system = 0;
    uint64_t idle = 0;
    uint64_t iowait = 0;
    uint64_t irq = 0;
    uint64_t softirq = 0;
    uint64_t steal = 0;

    uint64_t total() const noexcept {
        return user + nice + system + idle + iowait + irq + softirq + steal;
    }

    uint64_t idle_time() const noexcept {
        return idle + iowait;
    }
};

cpu_times read_cpu_times() {
    cpu_times times;
    std::ifstream proc("/proc/stat");
    if (!proc.is_open()) return times;

    std::string line;
    std::getline(proc, line);
    std::istringstream iss(line);
    std::string cpu_label;
    iss >> cpu_label >> times.user >> times.nice >> times.system
        >> times.idle >> times.iowait >> times.irq >> times.softirq >> times.steal;
    return times;
}

std::string read_line(const std::string path) {
    std::ifstream file(path);
    std::string line;
    return (file.is_open() && std::getline(file, line)) ? line : "";
};

}

system_info_reader_linux::system_info_reader_linux() noexcept {}

entity::cpu system_info_reader_linux::read_cpu() const {
    entity::cpu result;

    std::ifstream proc("/proc/cpuinfo");
    if (!proc.is_open()) {
        return result;
    }

    std::string line;
    /// Regex pattern: matches things like "AMD Ryzen 5 7430U"
    const std::regex amd_pattern(R"(AMD\s+Ryzen\s+\d+\s+\d+\w*)", std::regex::icase);
    const std::regex intel_pattern(R"(Intel\(R\)\s+.*\s+CPU\s+.*@.*GHz)", std::regex::icase);

    while(std::getline(proc, line)) {
        if (line.empty()) continue;

        const std::string value = extract_value(line);

        if (line.starts_with("model name")) {
            std::smatch match;
            if (std::regex_search(value, match, amd_pattern) || std::regex_search(value, match, intel_pattern)) {
                result.model_name = match.str();
            }
            else {
                result.model_name = value; /// unknow
            }
        }

        else if (line.starts_with("cpu cores")) {
            parse_number<uint16_t>(value, result.cpu_cores);
        }

        else if (line.starts_with("processor")) {
            parse_number<uint8_t>(value, result.processor_id);
        }

        else if (line.starts_with("cache size")) {
            parse_number<uint32_t>(value, result.l2_cache_kib);
        }

        else if (line.starts_with("cpu MHz")) {
            parse_number<float>(value,  result.frequency_mhz);
        }

        else if (line.starts_with("physical id")) {
            parse_number<uint16_t>(value, result.physical_id);
        }

        else if (line.starts_with("siblings")) {
            parse_number<uint16_t>(value,  result.siblings);
        }

        else if (line.starts_with("cpu cores")) {
            parse_number<uint16_t>(value, result.core_id);
        }
    }

    cpu_times t1 = read_cpu_times();
    std::this_thread::sleep_for(std::chrono::milliseconds(19));
    cpu_times t2 = read_cpu_times();

    const auto idle_diff = t2.idle_time() - t1.idle_time();
    const auto total_diff = t2.total() - t1.total();

    if (total_diff > 0) {
        result.usage_percent = 100.0 * (total_diff - idle_diff) / total_diff;
    }

    std::ifstream temp_file("/sys/class/hwmon/hwmon1/temp1_input");
    if (temp_file.is_open() && std::getline(temp_file, line)) {
        result.temperature_c = std::stoull(line);
    }

    return result;
}

entity::memory system_info_reader_linux::read_memory() const {
    entity::memory result;

    std::ifstream proc("/proc/meminfo");
    std::string line;
    uint64_t total_bytes = 0;
    uint64_t available_kb = 0;

    while (std::getline(proc, line)) {
        if (line.starts_with("MemTotal:")) {
            total_bytes = std::stoull(line.substr(9));
        }

        else if (line.starts_with("MemAvailable:")) {
            available_kb = std::stoull(line.substr(13));
        }

        if (total_bytes > 0 && available_kb > 0) break;
    }

    result.total_bytes = total_bytes / 1024;

    uint64_t available_mb = available_kb / 1024;

    result.used_bytes = result.total_bytes - available_mb;
    result.usage_percent = 100.0 * result.used_bytes / result.total_bytes;

    return result;
}

entity::gpu system_info_reader_linux::read_gpu() const {
    entity::gpu result;

    std::string value = read_line("/sys/class/drm/card1/device/uevent");
    if (!value.empty()) {
        result.name =  value.substr(value.find("=") + 1);
    }

    value = read_line("/sys/class/drm/card1/device/mem_info_vram_total");
    if (!value.empty()) {
        result.vram_total = (std::stoull(value) / 1024) / 1024;
    }

    value = read_line("/sys/class/drm/card1/device/mem_info_vram_used");
    if (!value.empty()) {
        result.vram_used = (std::stoull(value) / 1024) / 1024;
    }

    return result;
}

entity::disk system_info_reader_linux::read_disk() const {
    entity::disk result;

    std::string device = "nvme0n1";
    std::string str = "/sys/block/" + device + "/queue/hw_sector_size";
    std::string value = read_line(str.c_str());

    if (value.empty()) return result;
    result.sector_size = std::stoull(value);

    std::string str2 = "/sys/block/" + device + "/device/model";
    std::string value2 = read_line(str2.c_str());

    if (value2.empty()) return result;
    result.model = value2;

    auto fetch_disk_stats = [&device]() -> std::tuple<uint64_t, uint64_t> {
        std::ifstream proc("/proc/diskstats");
        if (!proc.is_open()) return {0,0};

        std::string line;
        std::vector<std::string> tokens;
        while (std::getline(proc, line)) {
            //find first line nvme0n1 string then push word into tokens vector
            if (line.find(device) != std::string::npos) {
                std::istringstream iss(line);

                std::string token;
                while (iss >> token)
                    tokens.push_back(token);

                if (tokens.size() > 9)
                    return {std::stoull(tokens[5]),std::stoull(tokens[9])};

                break;
            }
        }

        return {0,0};
    };

    auto [r1, w1] = fetch_disk_stats();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto [r2, w2] = fetch_disk_stats();
    //cal read && write speed of disk Mb/s
    result.read_speed = (r2 - r1) * result.sector_size / 1024.0 / 1024.0;
    result.write_speed = (w2 - w1) * result.sector_size / 1024.0 / 1024.0;

    return result;
}

entity::net system_info_reader_linux::read_net() const {
    entity::net result;

    uint64_t rx_byte = 0;
    uint64_t tx_byte = 0;

    std::string value = read_line("/sys/class/net/wlp1s0/statistics/rx_bytes");
    if (!value.empty()) {
        rx_byte = std::stoull(value);
    }

    value = read_line("/sys/class/net/wlp1s0/statistics/tx_bytes");
    if (!value.empty()) {
        rx_byte = std::stoull(value);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    value = read_line("/sys/class/net/wlp1s0/statistics/rx_bytes");
    if (!value.empty()) {
        result.rx_bytes = (std::stoull(value) - rx_byte) / 1024;
    }

    value = read_line("/sys/class/net/wlp1s0/statistics/tx_bytes");
    if (!value.empty()) {
        result.tx_bytes = (std::stoull(value) - tx_byte) / 1024;
    }

    return result;
}

} // namespace adapter
} // namespace adapter
