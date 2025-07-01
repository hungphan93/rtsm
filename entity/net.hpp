#ifndef ENTITY_NET_HPP
#define ENTITY_NET_HPP

#include <compare>
#include <cstdint>

namespace entity {

struct net {
    uint64_t rx_bytes = 0;
    uint64_t tx_bytes = 0;

    auto operator <=> (const net&) const = default;
};

} // namespace entity

#endif // ENTITY_NET_HPP
