#ifndef ENTITY_CPU_HPP
#define ENTITY_CPU_HPP

#include <string>
#include <cstdint>
#include <vector>

namespace entity {

struct cpu {
    struct core
    {
        /// core name string
        std::string name;

        /// a core usage percent in range [0.0f – 100.0f]
        float usage_percent = 0.0f;

        /// frequency mhz a core
        float frequency_mhz = 0.0f;

        /// temperature Celsius a core
        float temperature_c = 0.0f;
    };

    /// CPU cache size in KiB
    uint16_t cache_size = 0; // KiB?

    /// Power consumption in mW
    float power = 0.0f;

    /// CPU name string
    std::string name;

    /// List of core/thread info
    std::vector<core> threads;
};

} // namespace entity

#endif // ENTITY_CPU_HPP
