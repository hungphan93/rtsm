module;

#include <regex>
#include <cstdint>
#include <sys/statvfs.h>
#include <mntent.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <dlfcn.h> /// For libnvidia-ml.so dynamic loading

module adapter;
import std;

namespace adapter::linux2 {
/// Regex pattern: matches things like "AMD Ryzen 5 7430U"
static const std::regex cpu_name_pattern(
    R"(AMD\s+\w+(?:\s+\d+)?\s+\d+\w*|Intel\(R\)\s+.*\s+CPU\s+.*@.*GHz)",
    std::regex::icase | std::regex::optimize
    );

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

bool load_nvml() {
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
    entity::cpu result{};

    if (!cache_cpu_.initialized) {
        std::ifstream proc("/proc/cpuinfo");
        std::string line;

        while(std::getline(proc, line)) {
            auto value = detail::extract_value(line);

            if (line.contains("model name")) {
                std::cmatch match;
                if (std::regex_search(value.data(), value.data() + value.size(), match, cpu_name_pattern)) {
                    cache_cpu_.model_name = match.str();
                } else {
                    cache_cpu_.model_name = value; /// fallback: raw
                }
            }
            else if (line.contains("cache size")) detail::parse_first_number(cache_cpu_.l2_cache_kib, value);
            else if (line.contains("cpu cores")) detail::parse_first_number(cache_cpu_.cpu_cores, value);
            else if (line.contains("processor")) detail::parse_first_number(cache_cpu_.processor_id, value);
            else if (line.contains("physical id")) detail::parse_first_number(cache_cpu_.physical_id, value);
            else if (line.contains("siblings")) detail::parse_first_number(cache_cpu_.siblings, value);
            else if (line.contains("core id")) detail::parse_first_number(cache_cpu_.core_id, value);
        }

        /// temp cpu
        if (auto r = detail::find_hwmon_by_name("k10temp"); r) {
            cache_cpu_.hwmon_cpu_path = *r;
        }
        /// temp gpu
        if (auto r = detail::find_hwmon_by_name("amdgpu"); r) {
            cache_cpu_.hwmon_gpu_path = *r;
        }

        cache_cpu_.initialized = true;
    }

    result.model_name   = cache_cpu_.model_name;
    result.cpu_cores    = cache_cpu_.cpu_cores;
    result.processor_id = cache_cpu_.processor_id;
    result.l2_cache_kib = cache_cpu_.l2_cache_kib;
    result.physical_id  = cache_cpu_.physical_id;
    result.siblings     = cache_cpu_.siblings;
    result.core_id      = cache_cpu_.core_id;

    auto times_result = detail::read_cpu_times();
    if (!times_result) {
        return result;
    }

    const auto& current_times = *times_result;

    if (is_first_cpu_read_) {
        result.usage_percent = 0.0f;
        is_first_cpu_read_ = false;
    }
    else {
        const auto idle_diff  = current_times.idle_time() - last_cpu_times_.idle_time();
        const auto total_diff = current_times.total() - last_cpu_times_.total();

        if (total_diff > 0) {
            result.usage_percent = detail::percent(total_diff - idle_diff, total_diff);
        }
    }

    last_cpu_times_ = current_times;

    auto read_scaled_float = [](const fs::path& p, float scale) -> std::optional<float> {
        auto r = detail::parse_number<float>(detail::read_line(p));
        return r ? std::optional{*r / scale} : std::nullopt;
    };

    /// Read frequency mHz from the sysfs record using the read /proc/cpuinfo
    if (auto v = read_scaled_float("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", 1000.f); v) result.frequency_mhz = *v;

    /// Using Cache Path dynamically to read CPU temp from k10temp (not loop all folder)
    if (cache_cpu_.hwmon_cpu_path) {
        if (auto v = read_scaled_float(*cache_cpu_.hwmon_cpu_path / "temp1_input", 1000.f); v) result.temperature_c = *v;
    }

    /// Using Cache Path dynamically to read power from amdgpu if integrated APU
    if (cache_cpu_.hwmon_gpu_path) {
        if (auto v = read_scaled_float(*cache_cpu_.hwmon_gpu_path / "power1_input", 1.f); v) result.power_uw = *v;
    }

    return result;
}

entity::memory system_info_reader_linux::read_memory() const {
    entity::memory result{};

    /// https://docs.redhat.com/en/documentation/red_hat_enterprise_linux/6/html/deployment_guide/s2-proc-meminfo
    std::ifstream meminfo("/proc/meminfo");

    uint64_t total_kb = 0;
    uint64_t free_kb = 0;
    uint64_t available_kb = 0;
    std::string line;

    while (std::getline(meminfo, line)) {
        auto value = detail::extract_value(line);

        if (line.contains("MemTotal:")) detail::parse_first_number(total_kb, value);

        else if (line.contains("MemFree:")) detail::parse_first_number(free_kb, value);
        else if (line.contains("MemAvailable:")) detail::parse_first_number(available_kb,value);

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

    /// https://linux.die.net/man/8/dmidecode
    if (!memory_cache_.initialized) {
        std::string dmi_out = detail::exec_cmd("sudo -n dmidecode --type 17 2>/dev/null");

        std::istringstream stream(dmi_out);

        while (std::getline(stream, line)) {
            auto value = detail::extract_value(line);

            if (line.contains("Manufacturer:")) {
                memory_cache_.name = value;
            }

            else if (line.contains("Configured Memory Speed:")) {
                float raw_speed = 0;
                detail::parse_first_number(raw_speed, value);
                memory_cache_.frequency_mhz = raw_speed / 2;
            }

            else if (line.contains("Configured Voltage:") || line.contains("Maximum Voltage:")) {
                detail::parse_first_number(memory_cache_.voltage, value);
            }
        }

        if (!line.empty()) {
            /// Print warning to configure sudo privileges
            std::clog << "Please run \nsudo visudo\nusername ALL=(ALL) NOPASSWD: /usr/sbin/dmidecode\n";
        }

        memory_cache_.initialized = true;
    }

    /// data read only once
    result.name = memory_cache_.name;
    result.frequency_mhz = memory_cache_.frequency_mhz;
    result.voltage = memory_cache_.voltage;

    return result;
}

bool is_intel_integrated(uint16_t device_id) {
    /// Arc (Discrete) → 0x56A0 - 0x56FF
    if (device_id >= 0x56A0 && device_id <= 0x56FF) return false;

    return true; /// Vendor Intel + không phải Arc = iGPU
}

bool is_amd_integrated(uint16_t device_id) {
    /// APU / iGPU ranges
    if (device_id >= 0x1304 && device_id <= 0x131F) return true; /// Kabini APU
    if (device_id >= 0x1318 && device_id <= 0x131B) return true; /// Mullins APU
    if (device_id >= 0x15D8 && device_id <= 0x15DF) return true; /// Picasso / Raven2 (Ryzen 2000/3000G)
    if (device_id >= 0x15E7 && device_id <= 0x15FF) return true; /// Renoir (Ryzen 4000G)
    if (device_id >= 0x1636 && device_id <= 0x163F) return true; /// Renoir iGPU
    if (device_id >= 0x164C && device_id <= 0x164F) return true; /// Lucienne
    if (device_id >= 0x1681 && device_id <= 0x168F) return true; /// Rembrandt (Ryzen 6000) - RDNA2
    if (device_id >= 0x15BF && device_id <= 0x15C7) return true; /// Phoenix (Ryzen 7000) - RDNA3

    /// Discrete GPU ranges (Polaris, Vega, RDNA1/2/3)
    if (device_id >= 0x6600 && device_id <= 0x6FFF) return false; /// Polaris
    if (device_id >= 0x6800 && device_id <= 0x68FF) return false; /// Tahiti/GCN
    if (device_id >= 0x7300 && device_id <= 0x73FF) return false; /// RDNA2/3 dGPU

    return false; /// default is dGPU if not match
}

void system_info_reader_linux::classify_gpu(entity::gpu &result) const {
    switch (gpu_cache_.vendor) {

    case gpu_vendor::NVIDIA:
        read_nvidia_gpu(result);
        break;

    case gpu_vendor::AMD:
        is_amd_integrated(gpu_cache_.device_id)? read_amd_igpu(result) : read_amd_dgpu(result);

        break;

    case gpu_vendor::INTEL:
        break;

    /// Qualcomm Adreno on laptop ARM = iGPU
    case gpu_vendor::QUALCOMM:
        break;

    default:
        break;
    }
}

void system_info_reader_linux::read_nvidia_gpu(entity::gpu &result) const {
    /// Fast path for NVIDIA using NVML C API
    if (!nvml::load_nvml()) return;

    nvml::nvmlDevice_t device;
    if (nvml::nvmlDeviceGetHandle_ptr(0, &device) != nvml::NVML_SUCCESS) return;

    /// GPU name (written into cache so result.name stays up-to-date)
    if (nvml::nvmlDeviceGetName_ptr) {
        char name_buf[96] = {};
        if (nvml::nvmlDeviceGetName_ptr(device, name_buf, sizeof(name_buf)) == nvml::NVML_SUCCESS) {
            gpu_cache_.name = name_buf;
            result.name     = name_buf;
        }
    }

    /// VRAM
    nvml::nvmlMemory_t memory;
    if (nvml::nvmlDeviceGetMemoryInfo_ptr(device, &memory) == nvml::NVML_SUCCESS) {
        result.vram_total = memory.total / (1024 * 1024);
        result.vram_used  = memory.used  / (1024 * 1024);
        if (result.vram_total > 0) {
            result.usage_percent = detail::percent(result.vram_used, result.vram_total);
        }
    }

    /// Temperature  (0 = NVML_TEMPERATURE_GPU)
    unsigned int temp = 0;
    if (nvml::nvmlDeviceGetTemperature_ptr(device, 0, &temp) == nvml::NVML_SUCCESS) {
        result.temperature_c = temp;
    }

    /// Core clock  (0 = NVML_CLOCK_GRAPHICS)
    unsigned int freq = 0;
    if (nvml::nvmlDeviceGetClockInfo_ptr(device, 0, &freq) == nvml::NVML_SUCCESS) {
        result.frequency_mhz = freq;
    }
}

void system_info_reader_linux::read_amd_igpu(entity::gpu &result) const {

    if (auto v = detail::to_uint(detail::read_line(gpu_cache_.vram_used_path)); v) {
        result.vram_used    = *v / (1024 * 1024);
        result.usage_percent = detail::percent(result.vram_used, result.vram_total);
    }


    std::ifstream sclk_file(gpu_cache_.sclk_path);
    std::string line;

    while (std::getline(sclk_file, line)) {
        if (line.find('*') != std::string::npos) {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                auto value = detail::extract_value(line);
                detail::parse_first_number<uint64_t>(result.frequency_mhz, value);
            }
            break;
        }
    }

    if (auto temp_val = detail::to_uint(detail::read_line(gpu_cache_.temp_input_path)); temp_val && *temp_val > 0)
        result.temperature_c = *temp_val / 1000.f;

    if (auto pwr_val = detail::to_uint(detail::read_line(gpu_cache_.power_input_path)); pwr_val && *pwr_val > 0)
        result.power = *pwr_val / 1000000.f;
}

void system_info_reader_linux::read_amd_dgpu(entity::gpu& result) const {

}

entity::gpu system_info_reader_linux::read_gpu() const {
    entity::gpu result;

    if (!gpu_cache_.initialized) {
        fs::directory_iterator dir{"/sys/class/drm"};

        for (auto const& dir_entry : dir) {

            const std::string entry_name = dir_entry.path().filename().string();
            if (!entry_name.starts_with("card") || entry_name.size() < 5 || entry_name.size() > 6
                || !std::isdigit(static_cast<unsigned char>(entry_name[4]))) {
                continue;
            }

            const std::string vendor_str = detail::read_line(dir_entry.path() / "device/vendor");
            const std::string device_str = detail::read_line(dir_entry.path() / "device/device");

            const auto vendor_hex = detail::to_uint<uint16_t>(vendor_str, 16);
            const auto device_hex = detail::to_uint<uint16_t>(device_str, 16);
            if (!vendor_hex || !device_hex) continue;

            gpu_cache_.vendor    = static_cast<gpu_vendor>(*vendor_hex);
            gpu_cache_.device_id = *device_hex;
            gpu_cache_.drm_path  = dir_entry.path();

            if (gpu_cache_.vendor == gpu_vendor::NVIDIA) {
                gpu_cache_.name = "NVIDIA GPU";
            }
            else if (gpu_cache_.vendor == gpu_vendor::AMD) {
                gpu_cache_.name = "AMD Radeon Graphics";

                auto mem_total = detail::read_line(gpu_cache_.drm_path / "device/mem_info_vram_total");
                if (auto v = detail::to_uint(mem_total); v) gpu_cache_.vram_total = *v / (1024 * 1024);

                if (auto hwmon = detail::find_hwmon_by_name("amdgpu"); hwmon) {
                    gpu_cache_.hwmon_path = *hwmon;
                }

                gpu_cache_.vram_used_path   = gpu_cache_.drm_path / "device/mem_info_vram_used";
                gpu_cache_.sclk_path        = gpu_cache_.drm_path / "device/pp_dpm_sclk";
                gpu_cache_.temp_input_path  = gpu_cache_.hwmon_path / "temp1_input";
                gpu_cache_.power_input_path = gpu_cache_.hwmon_path / "power1_input";

            }
            else if (gpu_cache_.vendor == gpu_vendor::INTEL) {
                gpu_cache_.name = "Intel Graphics";
            }
            else {
                gpu_cache_.name = "Unknown GPU";
            }
        }

        gpu_cache_.initialized = true;
    }

    result.name = gpu_cache_.name;
    result.vram_total = gpu_cache_.vram_total;

    classify_gpu(result);

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
