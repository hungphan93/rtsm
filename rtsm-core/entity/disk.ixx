/// MIT License
export module entity:disk;

import std;

export namespace entity
{

struct disk {
	/// read speed of disk
	float read_speed = 0.0f;
	/// write speed of disk
	float write_speed = 0.0f;
	/// sector size of disk
	std::uint64_t sector_size = 0;
	/// name of disk
	std::string model = {};
	/// serial number of disk
	std::string serial_number = {};
	/// size of disk
	float size = 0.0f;
	/// swap usage
	float swap = 0.0f;
	/// used space
	float used = 0.0f;
	/// total space
	float total = 0.0f;
	/// free space
	float free = 0.0f;

	auto operator<=>(const disk &) const = default;
};

} /// namespace entity
