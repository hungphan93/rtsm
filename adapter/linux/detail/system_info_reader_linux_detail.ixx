/// MIT License
module;

#include <regex>

export module adapter:system_info_reader_linux_detail;

import std;
import util;

export namespace adapter::linux2::detail
{

namespace fs = std::filesystem;

/// Regex pattern: matches things like "AMD Ryzen 5 7430U"
const std::regex cpu_name_pattern(
	R"(AMD\s+\w+(?:\s+\d+)?\s+\d+\w*|Intel\(R\)\s+.*\s+CPU\s+.*@.*GHz)",
	std::regex::icase | std::regex::optimize);

struct cpu_times {
	std::uint64_t user = 0;
	std::uint64_t nice = 0;
	std::uint64_t system = 0;
	std::uint64_t idle = 0;
	std::uint64_t iowait = 0;
	std::uint64_t irq = 0;
	std::uint64_t softirq = 0;
	std::uint64_t steal = 0;

	inline std::uint64_t total() const noexcept
	{
		return user + nice + system + idle + iowait + irq + softirq + steal;
	}

	inline std::uint64_t idle_time() const noexcept
	{
		return idle + iowait;
	}
};

std::expected<cpu_times, std::errc> read_cpu_times()
{
	std::ifstream proc("/proc/stat");
	if (!proc.is_open()) {
		return std::unexpected(std::errc::no_such_file_or_directory);
	}

	std::string line;
	if (!std::getline(proc, line)) {
		return std::unexpected(std::errc::io_error);
	}

	/// only checking first line
	constexpr std::string_view prefix = "cpu ";
	if (!std::string_view(line).starts_with(prefix)) {
		return std::unexpected(std::errc::protocol_error);
	}

	cpu_times times{};
	const auto *first = line.data() + prefix.size();
	const auto *last = line.data() + line.size();

	for (auto out : { &times.user,
			  &times.nice,
			  &times.system,
			  &times.idle,
			  &times.iowait,
			  &times.irq,
			  &times.softirq,
			  &times.steal }) {
		/// ignoring character space
		while (first < last && *first == ' ')
			++first;

		auto [next, ec] = std::from_chars(first, last, *out);
		if (ec != std::errc{}) {
			return std::unexpected(ec);
		}
		first = next;
	}

	return times;
}

std::expected<std::string, std::errc> find_hwmon_by_name(std::string_view target)
{
	std::error_code ec;
	fs::directory_iterator it("/sys/class/hwmon/", ec);

	if (ec)
		return std::unexpected(std::errc::no_such_file_or_directory);

	for (const auto &entry : it) {
		auto name_path = entry.path() / "name";
		auto name = util::io::read_line(name_path);

		if (name == target) {
			return entry.path().string();
		}
	}

	return std::unexpected(std::errc::no_such_device);
}

std::expected<float, std::errc> read_cpu_frequency_avg(std::string_view filename) noexcept
{
	float sum = 0;
	int count = 0;

	while (true) {
		auto path = std::format("/sys/devices/system/cpu/cpu{}/cpufreq/{}",
					count++,
					filename);
		auto line = util::io::read_line(path);
		if (line.empty())
			break;

		if (auto v = util::parse::parse_number<float>(line); v) {
			sum += *v;
		}
	}

	if (count == 0)
		return std::unexpected(std::errc::no_such_device);

	return sum / count / 1000.f;
}

} /// adapter::linux::detail
