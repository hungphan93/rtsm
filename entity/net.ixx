/// MIT License
export module entity:net;

import std;

export namespace entity
{

struct net {
	std::uint64_t rx_bytes = 0;
	std::uint64_t tx_bytes = 0;

	auto operator<=>(const net &) const = default;
};

} /// namespace entity
