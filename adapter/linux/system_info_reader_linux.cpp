/// MIT License
#include "system_info_reader_linux.hpp"
#include <fstream>
#include <regex>
#include <thread>
#include <sys/statvfs.h>
#include <mntent.h>
#include <unordered_set>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace adapter {
namespace linux2 {

system_info_reader_linux::system_info_reader_linux() noexcept {}

entity::cpu system_info_reader_linux::read_cpu() const {
    entity::cpu result;

    if (!cache_cpu_.initialized) {
        std::ifstream proc("/proc/cpuinfo");
        std::string line;
        /// Regex pattern: matches things like "AMD Ryzen 5 7430U"
        const std::regex cpu_name_pattern(
            R"(AMD\s+Ryzen\s+\d+\s+\d+\w+\b|Intel\(R\)\s+.*\s+CPU\s+.*@.*GHz)",
            std::regex::icase | std::regex::optimize
            );

        while(proc.is_open() && std::getline(proc, line)) {
            if (line.empty()) continue;

            auto value = detail::trim(detail::extract_value(line));

            if (line.starts_with("model name")) {
                std::smatch match;
                std::string sv_str(value);
                if (std::regex_search(sv_str, match, cpu_name_pattern)) {
                    cache_cpu_.model_name = match.str();
                }
            }

            else if (line.starts_with("cpu cores")) {
                detail::parse_number(value, cache_cpu_.cpu_cores);
            }

            else if (line.starts_with("processor")) {
                detail::parse_number(value, cache_cpu_.processor_id);
            }

            else if (line.starts_with("cache size")) {
                detail::parse_number(value, cache_cpu_.l2_cache_kib);
            }

            else if (line.starts_with("physical id")) {
                detail::parse_number(value, cache_cpu_.physical_id);
            }

            else if (line.starts_with("siblings")) {
                detail::parse_number(value,  cache_cpu_.siblings);
            }

            else if (line.starts_with("core id")) {
                detail::parse_number(value, cache_cpu_.core_id);
            }

            cache_cpu_.hwmon_cpu_path = detail::find_hwmon_by_name("k10temp");
            cache_cpu_.hwmon_gpu_path = detail::find_hwmon_by_name("amdgpu");

            cache_cpu_.initialized = true;
        }
    }

    result.model_name   = cache_cpu_.model_name;
    result.cpu_cores    = cache_cpu_.cpu_cores;
    result.processor_id = cache_cpu_.processor_id;
    result.l2_cache_kib = cache_cpu_.l2_cache_kib;
    result.physical_id  = cache_cpu_.physical_id;
    result.siblings     = cache_cpu_.siblings;
    result.core_id      = cache_cpu_.core_id;

    detail::cpu_times current_times = detail::read_cpu_times();

    if (is_first_cpu_read_) {
        result.usage_percent = 0.0f;
        is_first_cpu_read_ = false;
    } else {
        const auto idle_diff = current_times.idle_time() - last_cpu_times_.idle_time();
        const auto total_diff = current_times.total() - last_cpu_times_.total();

        if (total_diff > 0) {
            result.usage_percent = detail::percent((total_diff - idle_diff), total_diff);
        }
    }

    last_cpu_times_ = current_times;
    std::string line;

    /// Read frequency mHz from the sysfs record using the read /proc/cpuinfo
    std::ifstream freq_file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    if (freq_file && std::getline(freq_file, line)) {
        float frequency = 0;
        detail::parse_number(line, frequency);
        result.frequency_mhz = frequency / 1000.0f;
    }

    /// Using Cache Path dynamically to read CPU temp from k10temp (not loop all folder)
    if (cache_cpu_.hwmon_cpu_path) {
        std::ifstream temp_file(*cache_cpu_.hwmon_cpu_path + "/temp1_input");
        if (temp_file && std::getline(temp_file, line)) {
            float temperature = 0;
            detail::parse_number(line, temperature);
            result.temperature_c = temperature / 1000.f;
        }
    }

    /// Using Cache Path dynamically to read power from amdgpu if integrated APU
    if (cache_cpu_.hwmon_gpu_path) {
        std::ifstream pwr_file(*cache_cpu_.hwmon_gpu_path + "/power1_input");
        if (pwr_file && std::getline(pwr_file, line)) {
            float power = 0;
            detail::parse_number(line, power);
            result.power_uw = power;
        }
    }

    return result;
}

entity::memory system_info_reader_linux::read_memory() const {
    entity::memory result{};

    std::ifstream meminfo("/proc/meminfo");
    if (!meminfo.is_open()) return result;

    std::string key;
    uint64_t val_kb = 0;
    std::string unit;

    uint64_t total_kb = 0;
    uint64_t free_kb = 0;
    uint64_t available_kb = 0;

    while (meminfo >> key >> val_kb >> unit) {
        if (key == "MemTotal:") {
            total_kb = val_kb;
        }

        else if (key == "MemFree:") {
            free_kb = val_kb;
        }

        else if (key == "MemAvailable:") {
            available_kb = val_kb;
        }

        if (total_kb > 0 && free_kb > 0 && available_kb > 0) {
            break;
        }
    }

    /// KB -> MB -> GB
    result.vram_total = total_kb / (1024.0f * 1024.0f);
    result.vram_free  = free_kb / (1024.0f * 1024.0f);

    float available_gb = available_kb / (1024.0f * 1024.0f);
    result.vram_used   = result.vram_total - available_gb;

    /// Using percent of RAM
    if (result.vram_used > 0 && result.vram_total > 0) {
        result.usage_percent = detail::percent(result.vram_used, result.vram_total);
    }

    if (!memory_cache_.initialized) {
        std::string dmi_out = detail::exec_cmd("sudo dmidecode --type 17 2>/dev/null");

        if (!dmi_out.empty()) {
            std::istringstream stream(dmi_out);
            std::string line;
            bool in_valid_slot = false;

            while (std::getline(stream, line)) {
                line = std::string(detail::trim(line));

                /// Start of a new memory device block
                if (line.starts_with("Memory Device")) {
                    in_valid_slot = true;
                }

                /// IF IT IS AN EMPTY SLOT -> Set flag to false to ignore all junk parameters below
                else if (line.starts_with("Size: No Module Installed")) {
                    in_valid_slot = false;
                }

                /// IF RAM IS INSTALLED IN THE SLOT -> Proceed to extract data and SAVE TO CACHE
                else if (in_valid_slot) {
                    std::string val;
                    /// 1. Manufacturer name
                    if (line.starts_with("Manufacturer:")) {
                        val = detail::trim(line.substr(13));
                        std::clog  <<"hung test 4= "<< val;
                        if (val != "Unknown" && val != "None") {
                            memory_cache_.name = val;
                        }
                    }

                    /// 2. Frequency (Configured Memory Speed or Speed)
                    else if (line.starts_with("Configured Memory Speed:") ||
                             (line.starts_with("Speed:"))) {
                        val = detail::trim(line.substr(line.find(':') + 1));
                        if (val != "Unknown" && val != "None") {
                            float raw_speed = 0.0f;
                            detail::parse_number(val, raw_speed);
                            /// dmidecode returns MT/s, divide by 2 to get actual MHz
                            memory_cache_.frequency_mhz = raw_speed / 2.0f;
                        }
                    }

                    /// 3. Voltage (Configured Voltage or Voltage)
                    else if (line.starts_with("Configured Voltage:")
                             || line.starts_with("Maximum Voltage:")
                             || line.starts_with("Voltage:")) {
                        val = std::string(detail::trim(line.substr(line.find(':') + 1)));
                        if (val != "Unknown" && val != "None") {
                            detail::parse_number(val, memory_cache_.voltage);
                        }
                    }
                }
            }
        }

        else {
            /// Print warning to configure sudo privileges
            std::clog << "Please run \nsudo visudo\nusername ALL=(ALL) NOPASSWD: /usr/sbin/dmidecode\n";
        }

        memory_cache_.initialized = true;
    }

    result.name = memory_cache_.name;
    result.frequency_mhz = memory_cache_.frequency_mhz;
    result.voltage = memory_cache_.voltage;

    return result;
}

bool amd_iGPU_devices(uint64_t device) {
    /// Vega iGPU (Ryzen 2000 to 5000)
    if ((device >= 0x15E0 && device <= 0x15EF) ||
        (device >= 0x1630 && device <= 0x163F))
        return true;

    /// RDNA2 iGPU (Ryzen 6000)
    if (device >= 0x1640 && device <= 0x168F)
        return true;

    /// RDNA3 iGPU (Ryzen 7000 to 8000)
    if ((device >= 0x1640 && device <= 0x165F) ||
        (device >= 0x16A0 && device <= 0x16AF))
        return true;

    /// RDNA3+ Strix Point iGPU
    if (device >= 0x1900 && device <= 0x19FF)
        return true;

    return false;
}

bool is_igpu_card(uint64_t vendor, uint64_t device) {
    if (vendor == 0x8086) return true;           /// Intel iGPU
    if (vendor == 0x1002) return amd_iGPU_devices(device); /// AMD iGPU
    return false;                                /// NVIDIA and other vendor is dGPU
}

entity::gpu system_info_reader_linux::read_gpu() const {
    entity::gpu result;

    if (!gpu_cache_.initialized) {
        std::error_code ec;
        fs::directory_iterator dir{"/sys/class/drm", ec};

        bool found_dgpu = false;

        if (!ec) {
            for (auto const& e : dir) {
                if (!e.is_directory(ec) || ec) continue;

                const std::string name = e.path().filename().string();
                if (!name.starts_with("card") || name.size() < 5 || name.size() > 6 || !std::isdigit(name[4])) {
                    continue;
                }

                const std::string vendor_str = detail::read_line(e.path() / "device/vendor");
                const std::string device_str = detail::read_line(e.path() / "device/device");

                if (vendor_str.empty() || device_str.empty()) continue;

                const auto vendor_hex = detail::to_uint(vendor_str, 16);
                const auto device_hex = detail::to_uint(device_str, 16);
                if (!vendor_hex || !device_hex) continue;

                bool igpu = is_igpu_card(*vendor_hex, *device_hex);

                if (gpu_cache_.drm_path.empty() || (!igpu && !found_dgpu)) {
                    gpu_cache_.drm_path = e.path().string();
                    gpu_cache_.is_nvidia = (*vendor_hex == 0x10de);
                    gpu_cache_.is_amd = (*vendor_hex == 0x1002);
                    gpu_cache_.is_intel = (*vendor_hex == 0x8086);

                    if (gpu_cache_.is_amd) gpu_cache_.name = "AMD Radeon Graphics";
                    else if (gpu_cache_.is_intel) gpu_cache_.name = "Intel Core Graphics";
                    else if (gpu_cache_.is_nvidia) gpu_cache_.name = "NVIDIA GeForce / RTX";

                    if (gpu_cache_.is_amd) {
                        auto mem_total = detail::read_line(e.path() / "device/mem_info_vram_total");
                        if (auto v = detail::to_uint(mem_total); v) gpu_cache_.vram_total = *v / (1024 * 1024);
                        gpu_cache_.hwmon_path = detail::find_hwmon_by_name("amdgpu");
                    }

                    if (!igpu) found_dgpu = true;
                }
            }
        }

        if (!found_dgpu) {
            const std::string nv_check = detail::exec_cmd("nvidia-smi --query-gpu=name --format=csv,noheader 2>/dev/null");
            if (!nv_check.empty() && nv_check.find("not found") == std::string::npos) {
                gpu_cache_.is_nvidia = true;
                gpu_cache_.name = std::string(detail::trim(nv_check.substr(0, nv_check.find('\n'))));
            }
        }

        gpu_cache_.initialized = true;
    }

    if (gpu_cache_.drm_path.empty() && !gpu_cache_.is_nvidia) return result;

    if (gpu_cache_.is_nvidia) {
        constexpr const char *cmd =
            "/usr/bin/nvidia-smi --query-gpu=memory.total,memory.used,temperature.gpu,clocks.gr "
            "--format=csv,noheader,nounits 2>/dev/null";

        const auto value = detail::exec_cmd(cmd);
        result.name = gpu_cache_.name;

        unsigned int total = 0, used = 0, temp = 0, freq = 0;

        if (std::sscanf(value.c_str(), "%u, %u, %u, %u", &total, &used, &temp, &freq) >= 2) {
            result.vram_total    = total;
            result.vram_used     = used;
            result.temperature_c = temp;
            result.frequency_mhz = freq;
            if (result.vram_total > 0) {
                result.usage_percent = detail::percent(result.vram_used, result.vram_total);
            }
        }
        return result;
    }

    result.name = gpu_cache_.name;
    result.vram_total = gpu_cache_.vram_total;

    if (!gpu_cache_.drm_path.empty()) {
        std::string value = detail::read_line(gpu_cache_.drm_path + "/device/mem_info_vram_used");
        if (auto v = detail::to_uint(value); v) {
            result.vram_used = *v / (1024 * 1024);
            if (result.vram_total > 0) {
                result.usage_percent = detail::percent(result.vram_used, result.vram_total);
            }
        }
    }

    if (gpu_cache_.is_amd && !gpu_cache_.drm_path.empty()) {
        std::ifstream sclk_file(gpu_cache_.drm_path + "/device/pp_dpm_sclk");
        std::string line;

        while (std::getline(sclk_file, line)) {
            if (line.find('*') != std::string::npos) {
                size_t colon_pos = line.find(':');
                if (colon_pos != std::string::npos) {
                    float freq = 0.0f;
                    std::sscanf(line.c_str() + colon_pos + 1, "%f", &freq);
                    result.frequency_mhz = freq;
                }
                break;
            }
        }

        if (gpu_cache_.hwmon_path) {
            std::ifstream temp_file(*gpu_cache_.hwmon_path + "/temp1_input");
            if (temp_file && std::getline(temp_file, line)) {
                auto temp_val = detail::to_uint(line).value_or(0);
                if (temp_val > 0) result.temperature_c = temp_val / 1000.f;
            }

            std::ifstream pwr_file(*gpu_cache_.hwmon_path + "/power1_input");
            if (pwr_file && std::getline(pwr_file, line)) {
                auto pwr_val = detail::to_uint(line).value_or(0);
                if (pwr_val > 0) result.power = pwr_val / 1000000.f;
            }
        }
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
            float available = 0;
            detail::parse_number(line.substr(10), available);
            swap_total_kb = available;
        }
        else if (line.starts_with("SwapFree:")) {
            float available = 0;
            detail::parse_number(line.substr(9), available);
            swap_free_kb = available;
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
    float sector = 0;
    detail::parse_number(value, sector);
    result.sector_size = sector;

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
            /// find first line nvme0n1 string then push word into tokens vector
            if (line.find(device) != std::string::npos) {
            std::istringstream iss(line);

            std::string token;
                while (iss >> token)
                tokens.push_back(token);

                if (tokens.size() > 9) {
                    uint64_t available1 = 0;
                    detail::parse_number(tokens[5], available1);

                    uint64_t available2 = 0;
                    detail::parse_number(tokens[9], available2);
                    return {available1, available2};
                }
                break;
            }
        }

        return {0,0};
    };

    auto [r1, w1] = fetch_disk_stats();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto [r2, w2] = fetch_disk_stats();
    /// cal read && write speed of disk Mb/s
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
        /// skip 2 header lines
        std::getline(proc, line);
        std::getline(proc, line);

        while (std::getline(proc, line)) {
            std::istringstream iss(line);
            std::string iface;
            uint64_t rx = 0, tx = 0;

            /// parse "<iface>:"
            std::getline(iss, iface, ':');
            iface.erase(0, iface.find_first_not_of(" \t"));
            iface.erase(iface.find_last_not_of(" \t") + 1);

            if (iface == "lo") continue;

            uint64_t skip;
            iss >> rx >> skip >> skip >> skip >> skip >> skip >> skip >> skip >> tx;

            if (rx > 0) return {rx, tx};
        }

        return {0, 0};
    };

    auto [rx0, tx0] = get_active_net_bytes();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto [rx1, tx1] = get_active_net_bytes();

    entity::net result;
    result.rx_bytes = (rx1 > rx0) ? (rx1 - rx0) / 1024 : 0; /// KB/s
    result.tx_bytes = (tx1 > tx0) ? (tx1 - tx0) / 1024 : 0;

    return result;
}

} /// namespace linux2
} /// namespace adapter
