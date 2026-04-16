/// MIT License
module;

#include <cstdint>

export module entity:disk;

import std;

export namespace entity {

struct disk {
    /// read speed of disk
    float read_speed = 0.0f;

    /// write speed of disk
    float write_speed = 0.0f;

    /// sictor size of disk
    uint64_t sector_size = 0;

    /// name of disk
    std::string model;

    /// serial number of disk
    std::string serial_number;

    /// size of disk
    float size = 0.0f;

    float swap = 0.0f;
    float used = 0.0f;
    float total = 0.0f;
    float free = 0.0f;

    auto operator <=> (const disk&) const = default;
};

} /// namespace entity
