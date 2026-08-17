/// MIT License
export module util:convert;

import std;

export namespace util::convert
{

float to_gb(float bytes) noexcept
{
	return bytes / (1024.0 * 1024.0 * 1024.0);
}

template <typename T>
requires std::is_arithmetic_v<T>
float percent(T used, T total) noexcept
{
	if (total == T{}) {
		return 0.0f;
	}

	return (100.0f * used) / total;
}

} /// namespace util::convert
