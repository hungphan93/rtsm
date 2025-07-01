#ifndef ENTITY_DISK_HPP
#define ENTITY_DISK_HPP

#include <string>
#include <cstdint>

namespace entity {

struct disk {
    /// read speed of disk
    uint64_t read_speed = 0;

    /// write speed of disk
    uint64_t write_speed = 0;

    /// sictor size of disk
    uint64_t sector_size = 0;

    /// name of disk
    std::string model;

    /// serial number of disk
    std::string serial_number;

    /// size of disk
    float size = 0.0f;

    auto operator <=> (const disk&) const = default;
};

} // namespace entity

#endif // ENTITY_DISK_HPP
