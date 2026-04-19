/// MIT License
module;

#include <cstdint>
#include <memory>

export module adapter:system_info_reader_linux_detail;

import std;

export namespace adapter::linux2::detail {

namespace fs = std::filesystem;

std::string_view trim(std::string_view s) noexcept {
    const auto start = s.find_first_not_of(" \t\n\r");
    if (start == std::string_view::npos) return {};
    const auto end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
};

std::string_view extract_value(const std::string_view line) noexcept {
    auto pos = line.find(':');

    if (pos == std::string_view::npos) {
        return {};
    }

    return trim(line.substr(pos + 1));
}


template<typename T>
    requires std::is_arithmetic_v<T>
std::expected<T, std::errc> parse_number(std::string_view text, int base = 10) noexcept {
    T out{};
    const auto first = text.data();
    const auto last = first + text.size();

    std::from_chars_result result;
    if constexpr (std::is_integral_v<T>) {
        result = std::from_chars(first, last, out, base);
    } else {
        result = std::from_chars(first, last, out);
    }

    if (result.ec != std::errc{}) {
        return std::unexpected(result.ec);
    }

    if (result.ptr != last) {
        return std::unexpected(std::errc::invalid_argument);
    }

    return out;
}

struct cpu_times {
    uint64_t user = 0;
    uint64_t nice = 0;
    uint64_t system = 0;
    uint64_t idle = 0;
    uint64_t iowait = 0;
    uint64_t irq = 0;
    uint64_t softirq = 0;
    uint64_t steal = 0;

    inline uint64_t total() const noexcept {
        return user + nice + system + idle + iowait + irq + softirq + steal;
    }

    inline uint64_t idle_time() const noexcept {
        return idle + iowait;
    }
};

std::expected<cpu_times, std::errc> read_cpu_times() {
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
    const auto* first = line.data() + prefix.size();
    const auto* last = line.data() + line.size();

    for (auto out : {&times.user, &times.nice, &times.system, &times.idle,
                     &times.iowait, &times.irq, &times.softirq, &times.steal}) {

        /// ignoring character space
        while (first < last && *first == ' ') ++first;

        auto [next, ec] = std::from_chars(first, last, *out);
        if (ec != std::errc{}) {
            return std::unexpected(ec);
        }
        first = next;
    }

    return times;
}

std::string read_line(const fs::path& path) {
    std::ifstream file(path);
    std::string line;
    std::getline(file, line);
    return line;
};

std::string exec_cmd(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    using pipe_close_t = int(*)(FILE*);
    std::unique_ptr<FILE, pipe_close_t> pipe(popen(cmd, "r"), static_cast<pipe_close_t>(pclose));
    if (!pipe) {
        std::clog << "popen() failed: " << cmd << "\n";
        return "";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

float to_gb(float bytes) {
    return bytes / (1024.0 * 1024.0 * 1024.0);
}

template<typename T>
    requires std::is_arithmetic_v<T>
float percent(T used, T total) noexcept {
    if (total == T{}) {
        return 0;
    }

    return (100.0 * used) / total;
}

std::expected<std::string, std::errc> find_hwmon_by_name(const std::string& target) {
    std::error_code ec;

    for (const auto& entry : fs::directory_iterator("/sys/class/hwmon", ec)) {
        if(read_line(entry.path() / "name") == target) {
            std::println("tét = {}", entry.path().string());
            return entry.path().string();
        }
    }

    return std::unexpected(std::errc::no_such_device);
}

/// Converting a uint string to uint64_t
template<typename T = uint64_t>
    requires std::is_arithmetic_v<T>
std::expected<T, std::errc> to_uint(std::string_view s, int base = 10) noexcept {
    if (s = trim(s); s.empty()) return std::unexpected(std::errc::invalid_argument);

    if (base == 16 || base == 0) {
        if (s.starts_with("0x") || s.starts_with("0X")) {
            s = s.substr(2);
            base = 16;
        }
    }

    return parse_number<T>(s, base);
}

} /// adapter::linux::detail

