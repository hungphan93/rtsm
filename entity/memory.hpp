#ifndef ENTITY_MEMORY_HPP
#define ENTITY_MEMORY_HPP
#include <cstdint>
#include <string>
namespace entity {

struct memory {
    uint64_t vram_total = 0;
    uint64_t vram_used = 0;
    uint64_t total_bytes = 0;
    uint64_t used_bytes = 0;
    std::string usage_percent;
    std::string name;
    double voltage = 0;
    double buss = 0;
    double swap = 0;
};

} // namespace entity

#endif // ENTITY_MEMORY_HPP
