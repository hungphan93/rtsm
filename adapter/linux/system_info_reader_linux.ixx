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
    bool is_nvidia = false;
    bool is_amd = false;
    bool is_intel = false;

    std::string name = "Unknown GPU";
    std::string drm_path; /// Cache path example: "/sys/class/drm/card0"
    std::expected<fs::path, std::errc> hwmon_path;

    uint64_t vram_total = 0;
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
};

} /// namespace adapter
