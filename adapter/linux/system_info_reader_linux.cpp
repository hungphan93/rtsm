#include "system_info_reader_linux.hpp"
#include <charconv>
#include <fstream>
#include <regex>

namespace adapter {
namespace linux2 {

namespace {

std::string extract_value(const std::string& line) {
    auto pos = line.find(':');
    if (pos == std::string::npos || pos + 2 >= line.size())
        return {};
    return line.substr(pos + 2);
}

template<typename T>
bool parse_number(const std::string& text, T& out) {
    auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), out);
    return ec == std::errc();
}

}

system_info_reader_linux::system_info_reader_linux() noexcept {}

entity::cpu system_info_reader_linux::read_cpu() const {
    entity::cpu result;

    std::ifstream proc("/proc/cpuinfo");
    if (!proc.is_open()) {
        return result;
    }

    std::string line;
    /// Regex pattern: matches things like "AMD Ryzen 5 7430U"
    const std::regex amd_pattern(R"(AMD\s+Ryzen\s+\d+\s+\d+\w*)", std::regex::icase);
    const std::regex intel_pattern(R"(Intel\(R\)\s+.*\s+CPU\s+.*@.*GHz)", std::regex::icase);

    while(std::getline(proc, line)) {
        if (line.empty()) continue;

        const std::string value = extract_value(line);

        if (line.starts_with("model name")) {
            std::smatch match;
            if (std::regex_search(value, match, amd_pattern) || std::regex_search(value, match, intel_pattern)) {
                result.model_name = match.str();
            }
            else {
                result.model_name = value; /// unknow
            }
        }

        else if (line.starts_with("cpu cores")) {
            parse_number<uint16_t>(value, result.cpu_cores);
        }

        else if (line.starts_with("processor")) {
            parse_number<uint8_t>(value, result.processor_id);
        }

        else if (line.starts_with("cache size")) {
            parse_number<uint32_t>(value, result.l2_cache_kib);
        }

        else if (line.starts_with("cpu MHz")) {
            parse_number<float>(value,  result.frequency_mhz);
        }

        else if (line.starts_with("physical id")) {
            parse_number<uint16_t>(value, result.physical_id);
        }

        else if (line.starts_with("siblings")) {
            parse_number<uint16_t>(value,  result.siblings);
        }

        else if (line.starts_with("cpu cores")) {
            parse_number<uint16_t>(value, result.core_id);
        }
    }

    return result;
}

entity::memory system_info_reader_linux::read_memory() const {
    entity::memory result;
    return result;
}

entity::gpu system_info_reader_linux::read_gpu() const {
    entity::gpu result;
    return result;
}

entity::disk system_info_reader_linux::read_disk() const {
    entity::disk result;
    return result;
}

entity::net system_info_reader_linux::read_net() const {
    entity::net result;
    return result;
}

} // namespace adapter
} // namespace adapter
