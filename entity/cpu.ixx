/// MIT License
module;

#include <cstdint>

export module entity:cpu;

import std;

export namespace entity {

struct cpu {
    /// Name cpu
    std::string model_name;

    /// Number of logical threads in this CPU (siblings)
    uint16_t siblings = 0;

    /// Local core ID within the physical CPU
    uint16_t core_id = 0;

    /// Number of physical cores in this CPU
    uint16_t cpu_cores = 0;

    /// A core usage percent in range [0.0f – 100.0f]
    float usage_percent = 0.0f;

    /// Frequency of this thread/core in MHz
    float frequency_mhz = 0.0f;

    /// Temperature of this thread/core in Celsius (if available)
    float temperature_c = 0.0f;

    /// Physical CPU identifier (0 for single-socket systems)
    uint16_t physical_id = 0;

    /// Cache sizes (in KiB)
    uint16_t l1_cache_kib = 0;
    uint32_t l2_cache_kib = 0;
    uint32_t l3_cache_kib = 0;

    /// Estimated power consumption (microwatt), if available
    float power_uw = 0.0f;

    /// Logical processor/thread ID
    uint8_t processor_id = 0;

    auto operator <=> (const cpu&) const = default;
};

} /// namespace entity
