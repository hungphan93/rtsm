/// MIT License
#ifndef ADAPTER_LINUX_SYSTEM_INFO_READER_LINUX_HPP
#define ADAPTER_LINUX_SYSTEM_INFO_READER_LINUX_HPP

#include "use_case/ports/system_info_reader.hpp"
#include "detail/system_info_reader_linux_detail.hpp"
#include <optional>

namespace adapter {
namespace linux2 {

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
    std::optional<std::string> hwmon_cpu_path;
    std::optional<std::string> hwmon_gpu_path;
};

struct memory_static_info {
    bool initialized = false;
    std::string name = "Ram";
    float frequency_mhz = 0.0f;
    float voltage = 0.0f;
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
};

} /// namespace linux2
} /// namespace adapter

#endif /// ADAPTER_LINUX_SYSTEM_INFO_READER_LINUX_HPP
