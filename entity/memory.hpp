#ifndef ENTITY_MEMORY_HPP
#define ENTITY_MEMORY_HPP

#include <cstdint>
#include <string>

namespace entity {

struct memory {
    float vram_free = 0.0f;
    float vram_total = 0.0f;
    float vram_used = 0.0f;
    float usage_percent =0.0f;
    uint64_t cached = 0;
    std::string name;
    float power_mw = 0.0f;
    float buss = 0.0f;
    float swap = 0.0f;
    float frequency_mhz = 0.0f;
    auto operator <=> (const memory&) const = default;
};

} // namespace entity

#endif // ENTITY_MEMORY_HPP
