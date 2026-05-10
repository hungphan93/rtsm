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

struct system_info_reader_linux : public usecase::system_info_reader {

    explicit system_info_reader_linux() noexcept;

    entity::cpu read_cpu() const override;
    entity::memory read_memory() const override;
    entity::gpu read_gpu() const override;
    entity::disk read_disk() const override;
    entity::net read_net() const override;

private:
    /// CPU
    mutable std::expected<fs::path, std::errc> hwmon_cpu_path_;
    mutable std::expected<fs::path, std::errc> hwmon_gpu_path_;
    mutable detail::cpu_times last_cpu_times_{};
    mutable std::ifstream proc_cpu_;

    /// RAM
    mutable std::ifstream proc_ram_;

    /// NET
    mutable uint64_t net_prev_rx_ = 0;
    mutable uint64_t net_prev_tx_ = 0;
    mutable std::chrono::steady_clock::time_point net_prev_t_;

    /// DISK
    mutable uint64_t disk_prev_r_ = 0;
    mutable uint64_t disk_prev_w_ = 0;
    mutable std::chrono::steady_clock::time_point disk_prev_t_;

    /// GPU
    void read_nvidia_gpu(entity::gpu& result) const;
    void read_amd_igpu(fs::path& hwmon_path, entity::gpu& result) const;
    void read_amd_dgpu(fs::path& hwmon_path, entity::gpu& result) const;
    void classify_gpu(fs::path& hwmon_path, entity::gpu &result) const;
};

} /// namespace adapter
