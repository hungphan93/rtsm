/// MIT License
module;

#include <cstdint>

export module entity:net;

export namespace entity {

struct net {
    uint64_t rx_bytes = 0;
    uint64_t tx_bytes = 0;

    auto operator <=> (const net&) const = default;
};

} /// namespace entity
