#ifndef ENTITY_MEMORY_HPP
#define ENTITY_MEMORY_HPP

#include <cstdint>
#include <string>

namespace entity {

struct memory {
    float vram_free = 0;
    float vram_total = 0;
    float vram_used = 0;
    float usage_percent =0.0f;
    uint64_t cached = 0;
    std::string name;
    double power_mw = 0;
    double buss = 0;
    double swap = 0;
    float frequency_mhz = 0.0f;
    auto operator <=> (const memory&) const = default;
};

} // namespace entity

#endif // ENTITY_MEMORY_HPP
