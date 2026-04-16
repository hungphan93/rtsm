/// MIT License
module;

#include <cstdint>

export module entity:gpu;

import std;

export namespace entity {

struct gpu {
    std::string name;
    uint64_t vram_total = 0;
    uint64_t vram_used = 0;
    uint64_t usage_percent = 0;
    uint64_t cores = 0;
    uint64_t frequency_mhz = 0;
    uint64_t temperature_c = 0;
    uint64_t power = 0;

    auto operator <=> (const gpu&) const = default;
};

} /// namespace entity
