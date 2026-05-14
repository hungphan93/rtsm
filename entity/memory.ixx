/// MIT License
module;

#include <cstdint>

export module entity:memory;

import std;

export namespace entity
{

struct memory {
	double      vram_free      = 0.0;
	double      vram_total     = 0.0;
	double      vram_available = 0.0;
	double      vram_used      = 0.0;
	float       usage_percent  = 0.0;
	uint64_t    cached         = 0;
	std::string name;
	float       voltage       = 0.0;
	float       buss          = 0.0;
	float       frequency_mhz = 0.0;

	auto operator<=>(const memory &) const = default;
};

} /// namespace entity
