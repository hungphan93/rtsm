#ifndef ADAPTER_SYSTEM_INFO_READER_LINUX_DETAIL_HPP
#define ADAPTER_SYSTEM_INFO_READER_LINUX_DETAIL_HPP

#include <charconv>
#include <filesystem>
#include <fstream>
#include <optional>

namespace adapter::linux2::detail {
namespace fs = std::filesystem;

inline std::string_view extract_value(const std::string_view line) {
    auto pos = line.find(':');
    return (pos == std::string_view::npos || pos + 2 >= line.size())
               ? "" : line.substr(pos + 2);
}

template<typename T>
inline bool parse_number(std::string_view text, T& out) {
    auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), out);
    return ec == std::errc();
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

inline cpu_times read_cpu_times() {
    cpu_times times;
    std::ifstream proc("/proc/stat");
    if (!proc.is_open()) return times;

    std::string line;
    std::getline(proc, line);
    std::istringstream iss(line);
    std::string cpu_label;
    iss >> cpu_label >> times.user >> times.nice >> times.system
        >> times.idle >> times.iowait >> times.irq >> times.softirq >> times.steal;
    return times;
}

inline std::string read_line(const std::string& path) {
    std::ifstream file(path);
    std::string line;
    return (file.is_open() && std::getline(file, line)) ? line : "";
};

inline std::string exec_cmd(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    using pipe_close_t = int(*)(FILE*);
    std::unique_ptr<FILE, pipe_close_t> pipe(popen(cmd, "r"), static_cast<pipe_close_t>(pclose));
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

inline std::string_view trim(std::string_view s) noexcept {
    const auto start = s.find_first_not_of(" \t\n\r");
    if (start == std::string_view::npos) return {};
    const auto end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
};

inline float to_gb(uint64_t bytes) {
    return bytes / (1024.0 * 1024.0 * 1024.0);
}

template<typename T>
inline float percent(T used, T total) {
    return (total > 0) ? (100.0 * used / total) : 0;
}

inline std::optional<std::string> find_hwmon_by_name(const std::string& target) {
    for (const auto& entry : fs::directory_iterator("/sys/class/hwmon")) {
        std::ifstream name_file(entry.path() / "name");
        std::string name;
        if (name_file && std::getline(name_file, name)) {
            if (name == target) {
                return entry.path().string();
            }
        }
    }
    return std::nullopt;
}

/// Converting a uint string to uint64_t
std::optional<uint64_t> to_uint(std::string_view s, int base = 10) noexcept {
    uint64_t value{};
    try {
        value = std::stoull(std::string(s), nullptr, base);
    } catch (...) {
        return std::nullopt;
    }
    return value;
}

} /// adapter::linux2::detail

#endif /// ADAPTER_SYSTEM_INFO_READER_LINUX_DETAIL_HPP
