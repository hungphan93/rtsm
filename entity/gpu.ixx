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
    uint16_t vendor = 0;
    uint16_t device = 0;
    uint32_t cores = 0;
    float usage_percent = 0.0f;
    float frequency_mhz = 0.0f;
    float temperature_c = 0.0f;
    float power = 0.0f;

    auto operator <=> (const gpu&) const = default;
};

} /// namespace entity
