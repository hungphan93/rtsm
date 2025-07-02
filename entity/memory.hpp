#ifndef ENTITY_MEMORY_HPP
#define ENTITY_MEMORY_HPP

#include <cstdint>
#include <string>

namespace entity {

struct memory {
    uint64_t vram_free = 0;
    uint64_t vram_total = 0;
    uint64_t vram_used = 0;
    float usage_percent =0.0f;
    uint64_t cached = 0;
    std::string name;
    double voltage = 0;
    double buss = 0;
    double swap = 0;
    auto operator <=> (const memory&) const = default;
};

} // namespace entity

#endif // ENTITY_MEMORY_HPP
