module;

#include <regex>
#include <cstdint>
#include <sys/statvfs.h>
#include <mntent.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <dlfcn.h> /// For libnvidia-ml.so dynamic loading

module adapter;
//#include "detail/system_info_reader_linux_detail.hpp"
import std;

namespace fs = std::filesystem;

namespace adapter::linux2 {

///=============================================================================
/// NVML Dynamic Loading Wrapper
///=============================================================================
namespace nvml {
    typedef enum nvmlReturn_enum { NVML_SUCCESS = 0 } nvmlReturn_t;
    typedef struct nvmlDevice_st* nvmlDevice_t;
    typedef struct nvmlMemory_st {
        unsigned long long total;
        unsigned long long free;
        unsigned long long used;
    } nvmlMemory_t;

    typedef nvmlReturn_t (*nvmlInit_t)(void);
    typedef nvmlReturn_t (*nvmlShutdown_t)(void);
    typedef nvmlReturn_t (*nvmlDeviceGetHandleByIndex_v2_t)(unsigned int, nvmlDevice_t*);
    typedef nvmlReturn_t (*nvmlDeviceGetMemoryInfo_t)(nvmlDevice_t, nvmlMemory_t*);
    typedef nvmlReturn_t (*nvmlDeviceGetTemperature_t)(nvmlDevice_t, int, unsigned int*);
    typedef nvmlReturn_t (*nvmlDeviceGetClockInfo_t)(nvmlDevice_t, int, unsigned int*);
    typedef nvmlReturn_t (*nvmlDeviceGetUtilizationRates_t)(nvmlDevice_t, void*); // Placeholder, actual struct needed
    typedef nvmlReturn_t (*nvmlDeviceGetName_t)(nvmlDevice_t, char*, unsigned int);
    typedef nvmlReturn_t (*nvmlDeviceGetCount_v2_t)(unsigned int*);


    static void* handle = nullptr;
    static bool loaded = false;
    static bool init_attempted = false;

    static nvmlInit_t nvmlInit_v2_ptr = nullptr;
    static nvmlDeviceGetHandleByIndex_v2_t nvmlDeviceGetHandle_ptr = nullptr;
    static nvmlDeviceGetUtilizationRates_t nvmlDeviceGetUtilizationRates_ptr = nullptr;
    static nvmlDeviceGetMemoryInfo_t nvmlDeviceGetMemoryInfo_ptr = nullptr;
    static nvmlDeviceGetClockInfo_t nvmlDeviceGetClockInfo_ptr = nullptr;
    static nvmlDeviceGetTemperature_t nvmlDeviceGetTemperature_ptr = nullptr;
    static nvmlDeviceGetName_t nvmlDeviceGetName_ptr = nullptr;
    static nvmlShutdown_t nvmlShutdown_ptr = nullptr;
    static nvmlDeviceGetCount_v2_t nvmlDeviceGetCount_v2_ptr = nullptr;


    struct nvml_guard {
        ~nvml_guard() {
            if (handle) {
                if (nvmlShutdown_ptr) {
                    nvmlShutdown_ptr();
                }
                dlclose(handle);
                handle = nullptr;
            }
        }
    };

    static nvml_guard guard;

    inline bool load_nvml() {
        if (init_attempted) return loaded;
        init_attempted = true;

        handle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
        if (!handle) handle = dlopen("libnvidia-ml.so", RTLD_LAZY);
        if (!handle) return false;

        nvmlInit_v2_ptr = (nvmlInit_t)dlsym(handle, "nvmlInit_v2");
        nvmlShutdown_ptr = (nvmlShutdown_t)dlsym(handle, "nvmlShutdown");
        nvmlDeviceGetHandle_ptr = (nvmlDeviceGetHandleByIndex_v2_t)dlsym(handle, "nvmlDeviceGetHandleByIndex_v2");
        nvmlDeviceGetMemoryInfo_ptr = (nvmlDeviceGetMemoryInfo_t)dlsym(handle, "nvmlDeviceGetMemoryInfo");
        nvmlDeviceGetTemperature_ptr = (nvmlDeviceGetTemperature_t)dlsym(handle, "nvmlDeviceGetTemperature");
        nvmlDeviceGetClockInfo_ptr = (nvmlDeviceGetClockInfo_t)dlsym(handle, "nvmlDeviceGetClockInfo");
        
        // Optional pointers (we don't fail if these are missing, as they are not currently used in read_gpu)
        nvmlDeviceGetUtilizationRates_ptr = (nvmlDeviceGetUtilizationRates_t)dlsym(handle, "nvmlDeviceGetUtilizationRates");
        nvmlDeviceGetName_ptr = (nvmlDeviceGetName_t)dlsym(handle, "nvmlDeviceGetName");
        nvmlDeviceGetCount_v2_ptr = (nvmlDeviceGetCount_v2_t)dlsym(handle, "nvmlDeviceGetCount_v2");

        if (!nvmlInit_v2_ptr || !nvmlShutdown_ptr || !nvmlDeviceGetHandle_ptr ||
            !nvmlDeviceGetMemoryInfo_ptr || !nvmlDeviceGetTemperature_ptr || !nvmlDeviceGetClockInfo_ptr) {
            dlclose(handle);
            handle = nullptr;
            loaded = false;
        } else if (nvmlInit_v2_ptr() == NVML_SUCCESS) {
            loaded = true;
        } else {
            dlclose(handle);
            handle = nullptr;
            loaded = false;
        }
        return loaded;
    }
}

system_info_reader_linux::system_info_reader_linux() noexcept {}

entity::cpu system_info_reader_linux::read_cpu() const {
    entity::cpu result;

    if (!cache_cpu_.initialized) {
        std::ifstream proc("/proc/cpuinfo");
        std::string line;
        /// Regex pattern: matches things like "AMD Ryzen 5 7430U"
        static const std::regex cpu_name_pattern(
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
        std::string dmi_out = detail::exec_cmd("sudo -n dmidecode --type 17 2>/dev/null");

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

    result.name = gpu_cache_.name;
    result.vram_total = gpu_cache_.vram_total;

    /// Fast path for NVIDIA using NVML C API
    if (gpu_cache_.is_nvidia && nvml::load_nvml()) {
        nvml::nvmlDevice_t device;
        if (nvml::nvmlDeviceGetHandle_ptr(0, &device) == nvml::NVML_SUCCESS) {
            nvml::nvmlMemory_t memory;
            if (nvml::nvmlDeviceGetMemoryInfo_ptr(device, &memory) == nvml::NVML_SUCCESS) {
                result.vram_total = memory.total / (1024 * 1024);
                result.vram_used = memory.used / (1024 * 1024);
                if (result.vram_total > 0) {
                    result.usage_percent = detail::percent(result.vram_used, result.vram_total);
                }
            }

            unsigned int temp = 0;
            /// 0 = NVML_TEMPERATURE_GPU
            if (nvml::nvmlDeviceGetTemperature_ptr(device, 0, &temp) == nvml::NVML_SUCCESS) {
                result.temperature_c = temp;
            }

            unsigned int freq = 0;
            /// 0 = NVML_CLOCK_GRAPHICS
            if (nvml::nvmlDeviceGetClockInfo_ptr(device, 0, &freq) == nvml::NVML_SUCCESS) {
                result.frequency_mhz = freq;
            }
        }
        return result;
    }

    /// Fallback to fallback methods for AMD / Intel
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

    float swap_total_kb = 0, swap_free_kb = 0;
    std::string line;

    for (std::ifstream meminfo("/proc/meminfo"); std::getline(meminfo, line); ) {
        if (line.starts_with("SwapTotal:")) {
            swap_total_kb = std::stoull(line.substr(10));
        } else if (line.starts_with("SwapFree:")) {
            swap_free_kb = std::stoull(line.substr(9));
        }
    }
    float swap_used_gb = (swap_total_kb - swap_free_kb) / (1024.0 * 1024.0);
    result.swap = swap_used_gb;

    std::unique_ptr<FILE, decltype(&endmntent)> mnt_file(setmntent("/proc/mounts", "r"), endmntent);
    if (mnt_file) {
        std::unordered_set<std::string> seen;
        struct mntent* mnt = nullptr;

        while ((mnt = getmntent(mnt_file.get())) != nullptr) {
            std::string fsname = mnt->mnt_fsname;
            std::string fstype = mnt->mnt_type;

            /// Skip pseudo filesystems, overlayfs, and known NETWORK/BLOCKING layers
            if (fsname.find("/dev/") != 0 || fsname.find("/dev/loop") == 0) continue;
            
            /// Critical Fix for Issue 2: Avoid statvfs freeze on disconnected network drives
            if (fstype == "nfs" || fstype == "nfs4" || fstype == "cifs" || fstype == "smb3" ||
                fstype == "autofs" || fstype.starts_with("fuse")) {
                continue;
            }

            if (!seen.insert(fsname).second) continue;

            struct statvfs stat_vfs;
            if (statvfs(mnt->mnt_dir, &stat_vfs) != 0) continue;

            double total_gb = detail::to_gb(static_cast<double>(stat_vfs.f_blocks) * static_cast<double>(stat_vfs.f_frsize));
            double free_gb  = detail::to_gb(static_cast<double>(stat_vfs.f_bavail) * static_cast<double>(stat_vfs.f_frsize));

            result.total += total_gb;
            result.free  += free_gb;
            result.used  += total_gb - free_gb;
        }
    }
    result.used += swap_used_gb;

    std::string device = "";
    struct ::stat root_stat;

    if (::stat("/", &root_stat) == 0) {
        unsigned int maj = major(root_stat.st_dev);
        unsigned int min = minor(root_stat.st_dev);
        std::string sys_path = "/sys/dev/block/" + std::to_string(maj) + ":" + std::to_string(min);

        std::error_code ec;
        auto target = fs::read_symlink(sys_path, ec);
        if (!ec) {
            std::string target_str = target.string();
            size_t block_pos = target_str.find("/block/");
            if (block_pos != std::string::npos) {
                size_t dev_start = block_pos + 7;
                size_t dev_end = target_str.find('/', dev_start);
                if (dev_end != std::string::npos) {
                    device = target_str.substr(dev_start, dev_end - dev_start);
                } else {
                    device = target_str.substr(dev_start);
                }
            }
        }
    }

    if (device.empty()) {
        std::error_code ec;
        for (const auto& entry : fs::directory_iterator("/sys/block", ec)) {
            if (ec) break;
            std::string name = entry.path().filename().string();
            if (!name.starts_with("loop") && !name.starts_with("ram") && !name.starts_with("sr")) {
                device = name;
                break;
            }
        }
        if (device.empty()) device = "sda";
    }

    std::string str_sector = "/sys/block/" + device + "/queue/hw_sector_size";
    std::string value_sector = detail::read_line(str_sector.c_str());
    result.sector_size = detail::to_uint(value_sector).value_or(512);

    std::string str_model = "/sys/block/" + device + "/device/model";
    result.model = detail::read_line(str_model.c_str());

    auto fetch_disk_stats = [&device]() -> std::tuple<uint64_t, uint64_t> {
        std::ifstream proc("/proc/diskstats");
        if (!proc.is_open()) return {0,0};

        std::string ln;
        while (std::getline(proc, ln)) {
            if (ln.find(device) == std::string::npos) continue;

            std::istringstream iss(ln);
            std::string token;
            int col = 0;
            bool is_target = false;
            uint64_t r_sect = 0, w_sect = 0;

            while (iss >> token) {
                if (col == 2) {
                    if (token == device) is_target = true;
                    else break;
                }
                if (is_target) {
                    if (col == 5) r_sect = detail::to_uint(token).value_or(0);
                    else if (col == 9) w_sect = detail::to_uint(token).value_or(0);
                }
                col++;
            }
            if (is_target && col > 9) return {r_sect, w_sect};
        }
        return {0,0};
    };

    auto current_time = std::chrono::steady_clock::now();
    auto [current_r, current_w] = fetch_disk_stats();

    auto& state = disk_cache_[device];

    if (!state.initialized) {
        state.r = current_r;
        state.w = current_w;
        state.t = current_time;
        state.initialized = true;

        result.read_speed = 0;
        result.write_speed = 0;
    } else {
        std::chrono::duration<double> dt = current_time - state.t;
        double dt_sec = dt.count();

        if (dt_sec > 0.001) {
            uint64_t diff_r = (current_r >= state.r) ? (current_r - state.r) : 0;
            uint64_t diff_w = (current_w >= state.w) ? (current_w - state.w) : 0;

            /// Kernel: Sector in diskstats is 512 Bytes
            result.read_speed = (diff_r * 512.0) / dt_sec;
            result.write_speed = (diff_w * 512.0) / dt_sec;
        } else {
            result.read_speed = 0;
            result.write_speed = 0;
        }

        // update Cache State
        state.r = current_r;
        state.w = current_w;
        state.t = current_time;
    }

    return result;
}

entity::net system_info_reader_linux::read_net() const {
    /// Zero-Allocation Parsing Logic
    auto get_total_net_bytes = []() -> std::pair<uint64_t, uint64_t> {
        std::ifstream proc("/proc/net/dev");
        if (!proc.is_open()) return {0, 0};

        std::string line;
        std::getline(proc, line);
        std::getline(proc, line);

        uint64_t total_rx = 0;
        uint64_t total_tx = 0;

        while (std::getline(proc, line)) {
            auto colon_pos = line.find(':');
            if (colon_pos == std::string::npos) continue;

            std::string_view iface_view(line.data(), colon_pos);
            auto start = iface_view.find_first_not_of(" \t");
            if (start != std::string_view::npos) {
                iface_view = iface_view.substr(start);
            }

            if (iface_view == "lo") continue;

            std::istringstream iss(line.substr(colon_pos + 1));
            uint64_t rx = 0, tx = 0, skip;

            if (iss >> rx >> skip >> skip >> skip >> skip >> skip >> skip >> skip >> tx) {
                total_rx += rx;
                total_tx += tx;
            }
        }
        return {total_rx, total_tx};
    };

    entity::net result;

    auto current_time = std::chrono::steady_clock::now();
    auto [current_rx, current_tx] = get_total_net_bytes();

    if (!net_cache_.initialized) {
        net_cache_.rx = current_rx;
        net_cache_.tx = current_tx;
        net_cache_.t = current_time;
        net_cache_.initialized = true;

        result.rx_bytes = 0;
        result.tx_bytes = 0;
    } else {
        std::chrono::duration<double> dt = current_time - net_cache_.t;
        double dt_sec = dt.count();

        /// calution Delta safe
        if (dt_sec > 0.001) {
            uint64_t diff_rx = (current_rx >= net_cache_.rx) ? (current_rx - net_cache_.rx) : 0;
            uint64_t diff_tx = (current_tx >= net_cache_.tx) ? (current_tx - net_cache_.tx) : 0;

            /// Raw Bytes/s
            result.rx_bytes = diff_rx / dt_sec;
            result.tx_bytes = diff_tx / dt_sec;
        } else {
            result.rx_bytes = 0;
            result.tx_bytes = 0;
        }

        /// update Cache State
        net_cache_.rx = current_rx;
        net_cache_.tx = current_tx;
        net_cache_.t = current_time;
    }

    return result;
}

} /// namespace
