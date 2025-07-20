#include "system_info_reader_linux.hpp"
#include <fstream>
#include <qlogging.h>
#include <regex>
#include <thread>
#include <sys/statvfs.h>
#include <mntent.h>
#include <unordered_set>
#include <qdebug.h>
#include "./detail/system_info_reader_linux_detail.hpp"

namespace adapter {
namespace linux2 {

system_info_reader_linux::system_info_reader_linux() noexcept {}

entity::cpu system_info_reader_linux::read_cpu() const {
    entity::cpu result;

    std::ifstream proc("/proc/cpuinfo");
    if (!proc.is_open()) {
        return result;
    }

    detail::cpu_times t1 = detail::read_cpu_times();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    detail::cpu_times t2 = detail::read_cpu_times();

    const auto idle_diff = t2.idle_time() - t1.idle_time();
    const auto total_diff = t2.total() - t1.total();

    if (total_diff > 0) {
        result.usage_percent = detail::percent((total_diff - idle_diff), total_diff);
    }

    std::string line;
    /// Regex pattern: matches things like "AMD Ryzen 5 7430U"
    const std::regex amd_pattern(R"(AMD\s+Ryzen\s+\d+\s+\d+\w+\b)", std::regex::icase);
    const std::regex intel_pattern(R"(Intel\(R\)\s+.*\s+CPU\s+.*@.*GHz)", std::regex::icase);

    while(std::getline(proc, line)) {
        if (line.empty()) continue;

        auto value = detail::trim(detail::extract_value(line));

        if (line.starts_with("model name")) {
            std::smatch match;
            std::string sv_str(value);
            if (std::regex_search(sv_str, match, std::regex(R"(AMD\s+Ryzen\s+\d+\s+\d+\w+\b|Intel\(R\)\s+.*\s+CPU\s+.*@.*GHz)", std::regex::icase))) {
                result.model_name = match.str();
            }
        }

        else if (line.starts_with("cpu cores")) {
            detail::parse_number(value, result.cpu_cores);
        }

        else if (line.starts_with("processor")) {
            detail::parse_number(value, result.processor_id);
        }

        else if (line.starts_with("cache size")) {
            detail::parse_number(value, result.l2_cache_kib);
        }

        else if (line.starts_with("cpu MHz")) {
            detail::parse_number(value,  result.frequency_mhz);
        }

        else if (line.starts_with("physical id")) {
            detail::parse_number(value, result.physical_id);
        }

        else if (line.starts_with("siblings")) {
            detail::parse_number(value,  result.siblings);
        }

        else if (line.starts_with("core id")) {
            detail::parse_number(value, result.core_id);
        }
    }

    /// dynamically find CPU temp from k10temp
    if (auto hwmon_cpu = detail::find_hwmon_by_name("k10temp")) {
        std::ifstream temp_file(*hwmon_cpu + "/temp1_input");
        if (temp_file && std::getline(temp_file, line)) {
            result.temperature_c = std::stoull(line);
        }
    }

    /// dynamically find power from amdgpu if integrated APU
    if (auto hwmon_gpu = detail::find_hwmon_by_name("amdgpu")) {
        std::ifstream pwr_file(*hwmon_gpu + "/power1_input");
        if (pwr_file && std::getline(pwr_file, line)) {
            result.power_mw = std::stoull(line);
        }
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
        result.usage_percent = detail::percent(result.vram_used, result.vram_total);
    }

    std::string value = detail::exec_cmd(
        "sudo -p '' dmidecode --type 17 2>/dev/null "
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

        result.name = detail::trim(line);

        iss >> line;
        detail::parse_number(detail::trim(line), result.frequency_mhz);

        iss >> line;
        detail::parse_number(detail::trim(line), result.power_mw);
    }

    return result;
}

entity::gpu system_info_reader_linux::read_gpu() const {
    entity::gpu result;

    std::string value = detail::read_line("/sys/class/drm/card1/device/uevent");
    if (!value.empty()) {
        result.name =  value.substr(value.find("=") + 1);
    }

    value = detail::read_line("/sys/class/drm/card1/device/mem_info_vram_total");
    if (!value.empty()) {
        result.vram_total = (std::stoull(value) / 1024);
    }

    value = detail::read_line("/sys/class/drm/card1/device/mem_info_vram_used");
    if (!value.empty()) {
        result.vram_used = (std::stoull(value) / 1024);
    }

    if (result.vram_used > 0) {
        result.usage_percent = detail::percent(result.vram_used, result.vram_total);
    }

    /// reading for nvidia card
    value = detail::exec_cmd("nvidia-smi --query-gpu=name,memory.total,memory.used,temperature.gpu --format=csv,noheader,nounits 2>/dev/null");
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
    std::string line;
    /// first try integrated AMD GPU info via amdgpu
    if (auto hwmon_gpu = detail::find_hwmon_by_name("amdgpu")) {
        std::ifstream temp_file(*hwmon_gpu + "/temp1_input");
        if (temp_file && std::getline(temp_file, line)) {
            result.temperature_c = std::stoull(line) / 1000.0;
        }

        std::ifstream pwr_file(*hwmon_gpu + "/power1_input");
        if (pwr_file && std::getline(pwr_file, line)) {
            result.power = std::stoull(line) / 1000.0;
        }
    }

    if (result.vram_used > 0) {
        result.usage_percent = detail::percent(result.vram_used, result.vram_total);
    }

    /// gpu get frequency mhz
    value = detail::exec_cmd(R"delim(cat /sys/class/drm/card1/device/pp_dpm_sclk | grep '\*' | awk '{print $2}' | sed 's/Mhz//')delim");
    if (!value.empty()) {
        result.frequency_mhz = std::stoull(value);
    }

    return result;
}

entity::disk system_info_reader_linux::read_disk() const {
    entity::disk result;

    /// read swap space used
    float swap_total_kb = 0, swap_free_kb = 0;
    std::string line;

    for (std::ifstream meminfo("/proc/meminfo"); std::getline(meminfo, line); ) {
        if (line.starts_with("SwapTotal:")) {
            swap_total_kb = std::stoull(line.substr(10));
        }
        else if (line.starts_with("SwapFree:")) {
            swap_free_kb = std::stoull(line.substr(9));
        }
    }

    float swap_used_gb = (swap_total_kb - swap_free_kb) / (1024.0 * 1024.0);
    result.swap = swap_used_gb;

    FILE* mnt_file = setmntent("/proc/mounts", "r");
    if (!mnt_file) return result;

    std::unordered_set<std::string> seen;
    struct mntent* mnt = nullptr;
    while ((mnt = getmntent(mnt_file)) != nullptr) {
        std::string fsname = mnt->mnt_fsname;
        if (fsname.find("/dev/") != 0 || fsname.find("/dev/loop") == 0) continue;
        if (!seen.insert(fsname).second) continue;

        struct statvfs stat;
        if (statvfs(mnt->mnt_dir, &stat) != 0) continue;

        /// /dev/nvme0n1p1
        /// /dev/nvme0n1p2
        /// /dev/nvme0n1p3
        /// /dev/nvme0n1p5
        double total_gb = detail::to_gb(stat.f_blocks * stat.f_frsize);
        double free_gb  = detail::to_gb(stat.f_bfree  * stat.f_frsize);
        result.total += total_gb;
        result.free  += free_gb;
        result.used  += total_gb - free_gb;
    }
    endmntent(mnt_file);

    result.used += swap_used_gb;

    std::string device = "nvme0n1";
    std::string str = "/sys/block/" + device + "/queue/hw_sector_size";
    std::string value = detail::read_line(str.c_str());

    if (value.empty()) return result;
    result.sector_size = std::stoull(value);

    std::string str2 = "/sys/block/" + device + "/device/model";
    std::string value2 = detail::read_line(str2.c_str());

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
