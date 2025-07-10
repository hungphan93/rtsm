#include "system_info_reader_linux.hpp"
#include <charconv>
#include <fstream>
#include <iostream>
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

std::string exec_cmd(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

void trim(std::string& s) {
    s.erase(0, s.find_first_not_of(" \t\n\r"));
    s.erase(s.find_last_not_of(" \t\n\r") + 1);
};

}

system_info_reader_linux::system_info_reader_linux() noexcept {}

entity::cpu system_info_reader_linux::read_cpu() const {
    entity::cpu result;

    std::ifstream proc("/proc/cpuinfo");
    if (!proc.is_open()) {
        return result;
    }

    cpu_times t1 = read_cpu_times();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    cpu_times t2 = read_cpu_times();

    const auto idle_diff = t2.idle_time() - t1.idle_time();
    const auto total_diff = t2.total() - t1.total();

    if (total_diff > 0) {
        result.usage_percent = 100.0 * (total_diff - idle_diff) / total_diff;
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
                if (!match.empty()) {
                    result.model_name = match[0];
                } else {
                    result.model_name = value;
                }
            }
            else {
                result.model_name = value;
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

        else if (line.starts_with("core id")) {
            parse_number<uint16_t>(value, result.core_id);
        }
    }

    std::ifstream temp_file("/sys/class/hwmon/hwmon1/temp1_input");
    if (temp_file.is_open() && std::getline(temp_file, line)) {
        result.temperature_c = std::stoull(line);
    }

    /// for chip amd onboard
    std::ifstream gpu_pwr("/sys/class/hwmon/hwmon7/power1_input");
    if (gpu_pwr.is_open() && std::getline(gpu_pwr, line)) {
        result.power_mw = std::stoull(line);
    }

    return result;
}

entity::memory system_info_reader_linux::read_memory() const {
    entity::memory result;

    std::ifstream proc("/proc/meminfo");
    std::string line;
    float total_bytes = 0;
    float available_kb = 0;

    while (std::getline(proc, line)) {
        if (line.starts_with("MemTotal:")) {
            total_bytes = std::stoull(line.substr(9));
        }

        else if (line.starts_with("MemAvailable:")) {
            available_kb = std::stoull(line.substr(13));
        }

        else if (line.starts_with("MemFree:")) {
            result.vram_free = std::stoull(line.substr(8));
        }

        if (total_bytes > 0 && available_kb > 0) break;
    }

    result.vram_total = total_bytes / 1024 / 1024;

    float available_mb = available_kb / 1024 / 1024;

    result.vram_used = result.vram_total - available_mb;

    if (result.vram_used > 0) {
        result.usage_percent = (100 * result.vram_used) / result.vram_total;
    }

    std::string value = exec_cmd(
        "cat /home/hungphan/password.txt | sudo -S -p '' dmidecode --type 17 2>/dev/null "
        "| grep -E 'Manufacturer:|Configured Memory Speed:|Voltage' "
        "| grep -v 'Unknown' "
        "| awk -F: '{print $2}' "
        "| sed 's/ MT\\/s//' "
        "| sed 's/ V//' "
        "| head -n 3"
        );

    if (!value.empty()) {
        std::istringstream iss(value);

        std::getline(iss, line);

        trim(line);
        result.name = line;

        iss >> line;
        parse_number(line, result.frequency_mhz);

        iss >> line;
        parse_number(line, result.power_mw);
    }

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
        result.vram_total = (std::stoull(value) / 1024);
    }

    value = read_line("/sys/class/drm/card1/device/mem_info_vram_used");
    if (!value.empty()) {
        result.vram_used = (std::stoull(value) / 1024);
    }

    if (result.vram_used > 0) {
        result.usage_percent = (100 * result.vram_used) / result.vram_total;
    }

    /// reading for nvidia card
    value = exec_cmd("nvidia-smi --query-gpu=name,memory.total,memory.used,temperature.gpu --format=csv,noheader,nounits 2>/dev/null");
    /// value = exec_cmd("nvidia-smi --query-gpu=name,memory.total,memory.used,temperature.gpu --format=csv,noheader,nounits");
    if (!value.empty()) {
        // Example line: GeForce RTX 3060, 12288, 2205, 37
        std::istringstream iss(value);
        std::string text;
        std::getline(iss, text,',');

        auto trim = [](std::string& s) {
            s.erase(0, s.find_first_not_of(" \t\n\r"));
            s.erase(s.find_last_not_of(" \t\n\r") + 1);
        };

        trim(text);
        result.name = text;

        iss.ignore(1); // skip comma

        iss >> text;
        trim(text);
        result.vram_total = std::stoull(text);

        iss.ignore(1); // skip comma

        iss >> text;
        trim(text);
        result.vram_used = std::stoull(text);

        iss >> text;
        trim(text);
        result.temperature_c = std::stoull(text);
    }

    /// for gpu amd onboard
    value = read_line("/sys/class/hwmon/hwmon7/temp1_input");
    if (!value.empty()) {
        result.temperature_c = std::stoull(value) / 1000;
    }

    if (result.vram_used > 0) {
        result.usage_percent = 100 * result.vram_used / result.vram_total;
    }

    /// gpu get frequency mhz
    value = exec_cmd(R"delim(cat /sys/class/drm/card1/device/pp_dpm_sclk | grep '\*' | awk '{print $2}' | sed 's/Mhz//')delim");
    if (!value.empty()) {
        result.frequency_mhz = std::stoull(value);
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
                    return {std::stoull(tokens[5]), std::stoull(tokens[9])};

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
    auto get_active_net_bytes = []() -> std::pair<uint64_t, uint64_t> {
        std::ifstream proc("/proc/net/dev");
        if (!proc.is_open()) {
            return {};
        }

        std::string line;
        // skip 2 header lines
        std::getline(proc, line);
        std::getline(proc, line);

        while (std::getline(proc, line)) {
            std::istringstream iss(line);
            std::string iface;
            uint64_t rx = 0, tx = 0;

            // parse "<iface>:"
            std::getline(iss, iface, ':');
            iface.erase(0, iface.find_first_not_of(" \t"));
            iface.erase(iface.find_last_not_of(" \t") + 1);

            if (iface == "lo") {
                continue;
            }

            uint64_t skip;
            iss >> rx >> skip >> skip >> skip >> skip >> skip >> skip >> skip >> tx;

            if (rx > 0) {
                return {rx, tx};
            }
        }

        return {0, 0};
    };

    auto [rx0, tx0] = get_active_net_bytes();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto [rx1, tx1] = get_active_net_bytes();

    entity::net result;
    result.rx_bytes = (rx1 > rx0) ? (rx1 - rx0) / 1024 : 0; // KB/s
    result.tx_bytes = (tx1 > tx0) ? (tx1 - tx0) / 1024 : 0;

    return result;
}

} // namespace adapter
} // namespace adapter
