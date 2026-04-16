/// MIT License
module;

export module usecase:system_info_reader;

import entity;

export namespace usecase {

struct system_info_reader {

    virtual ~system_info_reader() = default;

    /// Read current CPU info snapshot
    virtual entity::cpu read_cpu() const = 0;

    /// Read current disk usage and stats
    virtual entity::disk read_disk() const = 0;

    /// Read current GPU info (if available)
    virtual entity::gpu read_gpu() const = 0;

    /// Read current memory (RAM) usage
    virtual entity::memory read_memory() const = 0;

    /// Read current network statistics
    virtual entity::net read_net() const = 0;
};

} /// namespace usecase