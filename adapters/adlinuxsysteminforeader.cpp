#include "adlinuxsysteminforeader.h"
#include "entities/ecpuinfo.h"
#include "entities/esysteminfo.h"
#include <system_error>
#include <iostream>
#include <QDebug>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <regex>

ADLinuxSystemInfoReader::ADLinuxSystemInfoReader()
{
    qDebug() << "hung phan";
}

ESystemInfo& ADLinuxSystemInfoReader::read()
{
    info.cpu = readCpuInfoFromProc();
    info.cpu.threads.front().temperatureC = readCpuTempFromSys();
    info.mem = readMemoryInfo();
    return info;
}

std::optional<int> ADLinuxSystemInfoReader::parseInt(const std::string& s) {
    try {
        return std::stoi(s);
    } catch (...) {
        std::cerr << "Failed to parse number" << std::endl;
        return std::nullopt;
    }
}

ECpuInfo ADLinuxSystemInfoReader::readCpuInfoFromProc()
{
    std::ifstream cpuinfo("/proc/cpuinfo");

    if (!cpuinfo.is_open())
    {
        std::cerr << "Can't open /proc/cpuinfo file "
                  << std::system_category().message(errno) << std::endl;
        return {};
    }

    ECpuInfo cpu;
    ECpuCore core;
    std::string line;

    while (std::getline(cpuinfo, line))
    {
        if (line.starts_with("model name"))
        {
            // Regex pattern: matches things like "AMD Ryzen 5 7430U"
            std::regex cpuPattern(R"(AMD\s+Ryzen\s+\d+\s+\d+\w*)", std::regex_constants::icase);

            std::smatch match;
            std::string text = line.substr(line.find(":") + 2);

            if (std::regex_search(text, match, cpuPattern)) {
                std::cerr << "CPU Detected: " << match.str() << std::endl;
            } else {
                std::cerr << "CPU not found in text." << std::endl;
            }

            cpu.modelName = match.str();
        }

        if (line.starts_with("cpu cores"))
        {
            auto result = parseInt(line.substr(line.find(":") + 2));

            if (result)
            {
                cpu.coreNumber = *result;
            }
            else
            {
                std::cerr << "Can't parse string to int "  << std::endl;
                break;
            }
        }

        if (line.starts_with("processor"))
        {
            core.name = line.substr(line.find(":") + 2);
        }

        if (line.starts_with("cache size"))
        {
            core.cacheSize = line.substr(line.find(":") + 2);
        }

        if (line.starts_with("cpu MHz"))
        {
            core.frequencyMhz = line.substr(line.find(":") + 2);
        }

        if (line.starts_with("power management"))
        {
            core.usagePercent = getCpuUsagePercent();
            cpu.threads.push_back(core);
            core = {};
        }
    }

    return cpu;
}

std::string ADLinuxSystemInfoReader::readCpuTempFromSys()
{
    namespace fs = std::filesystem;
    const fs::path thermalDir = "/sys/class/thermal";

    if (!fs::exists(thermalDir) || !fs::is_directory(thermalDir)) {
        std::cerr << "Thermal directory does not exist or is not accessible: " << thermalDir << '\n';
        return "";
    }

    for (const auto& entry : fs::directory_iterator(thermalDir))
    {
        const auto& path = entry.path();
        const auto& name = path.filename().string();

        if (entry.is_directory() && name.rfind("thermal_zone", 0) == 0)
        {
            std::ifstream tempFile(path / "temp");
            if (tempFile)
            {
                std::string line;
                std::getline(tempFile, line);
                return line;
            }
        }
    }

    std::cerr << "No readable thermal_zone*/temp file found in " << thermalDir << '\n';
    return "";
}

CpuTimes ADLinuxSystemInfoReader::readCpuTimes()
{
    std::ifstream file("/proc/stat");
    std::string line;
    std::getline(file, line);

    std::istringstream iss(line);
    std::string cpu_label;
    CpuTimes times;
    iss >> cpu_label >> times.user >> times.nice >> times.system >> times.idle
        >> times.iowait >> times.irq >> times.softirq >> times.steal;

    return times;
}

std::string ADLinuxSystemInfoReader::getCpuUsagePercent() {
    CpuTimes t1 = readCpuTimes();
    std::this_thread::sleep_for(std::chrono::microseconds(800));
    CpuTimes t2 = readCpuTimes();

    unsigned long long idleDiff = t2.idleTime() - t1.idleTime();
    unsigned long long totalDiff = t2.total() - t1.total();

    if (totalDiff == 0) return "0";

    double usage = 100.0 * (totalDiff - idleDiff) / totalDiff;
    char buf[10];
    snprintf(buf, sizeof(buf), "%.1f%", usage);
    return std::string(buf);
}

EMemoryInfo ADLinuxSystemInfoReader::readMemoryInfo() {
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    uint64_t total_bytes = 0, availableKb = 0;

    while (std::getline(meminfo, line)) {
        if (line.starts_with("MemTotal:")) {
            total_bytes = std::stoull(line.substr(9));
        } else if (line.starts_with("MemAvailable:")) {
            availableKb = std::stoull(line.substr(13));
        }

        if (total_bytes > 0 && availableKb > 0) break;
    }

    EMemoryInfo mem;
    mem.total_bytes = total_bytes / 1024;
    uint64_t availableMb = availableKb / 1024;
    mem.used_bytes = mem.total_bytes - availableMb;

    double percent = 100.0 * mem.used_bytes / static_cast<double>(mem.total_bytes);
    char buf[10];
    snprintf(buf, sizeof(buf), "%.1f%", percent);
    mem.usage_percent = std::string(buf);

    return mem;
}
