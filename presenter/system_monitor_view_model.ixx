/// MIT License
export module presenter:system_monitor_view_model;

import std;

export namespace presenter {

struct cpu_view_model {
    std::string model_name;
    std::string usage_percent;
    std::string frequency_mhz;
    std::string temperature_c;
    std::string power;

    auto operator <=> (const cpu_view_model&) const = default;
};

struct memory_view_model {
    std::string name;
    std::string vram_total;
    std::string vram_used;
    std::string usage_percent;
    std::string voltage;
    std::string frequency_mhz;

    auto operator <=> (const memory_view_model&) const = default;
};

struct gpu_view_model {
    std::string name;
    std::string vram_total;
    std::string vram_used;
    std::string usage_percent;
    std::string cores;
    std::string frequency_mhz;
    std::string temperature_c;

    auto operator <=> (const gpu_view_model&) const = default;
};

struct disk_view_model {
    std::string model;
    std::string serial_number;
    std::string size;
    std::string read_speed;
    std::string write_speed;
    std::string usage_percent;
    std::string sector_size;

    auto operator <=> (const disk_view_model&) const = default;
};

struct net_view_model {
    std::string rx_speed;
    std::string tx_speed;

    auto operator <=> (const net_view_model&) const = default;
};

} /// namespace presenter
