/// MIT
export module util:parse;

import std;

export namespace util::parse
{

std::string_view trim(std::string_view s) noexcept
{
	const auto start = s.find_first_not_of(" \t\n\r");
	if (start == std::string_view::npos)
		return {};

	const auto end = s.find_last_not_of(" \t\n\r");
	return s.substr(start, end - start + 1);
};

std::string_view extract_value(std::string_view line) noexcept
{
	auto pos = line.find(':');

	if (pos == std::string_view::npos) {
		return {};
	}

	return trim(line.substr(pos + 1));
}

template <typename T>
requires std::is_arithmetic_v<T>
std::expected<T, std::errc> parse_number(std::string_view text, int base = 10) noexcept
{
	T out{};
	const auto first = text.data();
	const auto last = first + text.size();
	std::from_chars_result result;

	if constexpr (std::is_integral_v<T>) {
		result = std::from_chars(first, last, out, base);
	} else {
		result = std::from_chars(first, last, out);
	}

	if (result.ec != std::errc{} || result.ptr != last) {
		return std::unexpected(result.ec);
	}

	return out;
}

template <typename T>
requires std::is_arithmetic_v<T>
void parse_first_number(T &field, std::string_view s, int base = 10) noexcept
{
	auto start = s.find_first_not_of(' ');
	if (start == std::string_view::npos)
		return;

	s = s.substr(start);
	/// "16384 kB" → "16384", "1234Mhz" → "1234"
	s = s.substr(0, s.find_first_not_of("0123456789."));

	if (auto r = parse_number<T>(s, base); r)
		field = *r;
}

/// Converting a uint string to uint64_t
template <typename T = std::uint64_t>
requires std::is_arithmetic_v<T>
std::expected<T, std::errc> to_uint(std::string_view s, int base = 10) noexcept
{
	if (s = trim(s); s.empty())
		return std::unexpected(std::errc::invalid_argument);

	if (base == 16 || base == 0) {
		if (s.starts_with("0x") || s.starts_with("0X")) {
			s = s.substr(2);
			base = 16;
		}
	}

	return parse_number<T>(s, base);
}

} /// namespace util::parse
