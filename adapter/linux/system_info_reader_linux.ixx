/// MIT License
module;

#include <cstdint>

export module adapter:system_info_reader_linux;

import usecase;
import entity;
import :system_info_reader_linux_detail;
import std;

namespace fs = std::filesystem;

export namespace adapter::linux2 {

enum class gpu_vendor : uint16_t {
    NVIDIA   = 0x10DE,
    AMD      = 0x1002,
    INTEL    = 0x8086,
    QUALCOMM = 0x17CB,
    UNKNOWN  = 0x0000
};

struct cpu_static_info {
    bool initialized = false; /// flag for lazy initialization

    std::string model_name;
    uint16_t siblings = 0;
    uint16_t core_id = 0;
    uint16_t cpu_cores = 0;
    uint16_t physical_id = 0;
    uint8_t processor_id = 0;
    uint32_t l2_cache_kib = 0;

    /// Caching path hwmon O(1)
    std::expected<fs::path, std::errc> hwmon_cpu_path;
    std::expected<fs::path, std::errc> hwmon_gpu_path;
};

struct memory_static_info {
    bool initialized = false;
    std::string name = "Ram";
    float frequency_mhz = 0.0f;
    float voltage = 0.0f;
};

struct gpu_static_info {
    bool initialized = false;
    gpu_vendor vendor;
    uint16_t device_id;

    std::string name;
    uint64_t vram_total = 0;

    fs::path hwmon_path;
    fs::path drm_path; /// Cache path example: "/sys/class/drm/card0"
    fs::path vram_used_path;
    fs::path sclk_path;
    fs::path temp_input_path;
    fs::path power_input_path;
};

struct disk_static_info {
    uint64_t r = 0;
    uint64_t w = 0;
    std::chrono::steady_clock::time_point t;
    bool initialized = false;
};

struct net_static_info {
    uint64_t rx = 0;
    uint64_t tx = 0;
    std::chrono::steady_clock::time_point t;
    bool initialized = false;
};

struct system_info_reader_linux : public usecase::system_info_reader {

    explicit system_info_reader_linux() noexcept;

    entity::cpu read_cpu() const override;

    entity::memory read_memory() const override;

    entity::gpu read_gpu() const override;

    entity::disk read_disk() const override;

    entity::net read_net() const override;

private:
    mutable cpu_static_info cache_cpu_{};
    mutable detail::cpu_times last_cpu_times_{};
    mutable bool is_first_cpu_read_{true};

    mutable memory_static_info memory_cache_{};
    mutable gpu_static_info gpu_cache_{};
    mutable net_static_info net_cache_{};
    mutable std::unordered_map<std::string, disk_static_info> disk_cache_{};

    void read_nvidia_gpu(entity::gpu& result) const;
    void read_amd_igpu(entity::gpu& result) const;
    void read_amd_dgpu(entity::gpu& result) const;
    void classify_gpu(entity::gpu &result) const;
};

} /// namespace adapter
