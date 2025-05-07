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

ADLinuxSystemInfoReader::ADLinuxSystemInfoReader(): info{}
{

}
ADLinuxSystemInfoReader::~ADLinuxSystemInfoReader()
{
}
ESystemInfo& ADLinuxSystemInfoReader::read()
{
    std::thread([this]() {
        readCpuInfoFromProc();
        readNetworkInfo();
    }).detach();
    readCpuTempFromSys();
    readMemoryInfo();
    readGpuInfo();
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

void ADLinuxSystemInfoReader::readCpuInfoFromProc()
{
    std::ifstream cpuinfo("/proc/cpuinfo");

    if (!cpuinfo.is_open())
    {
        std::cerr << "Can't open /proc/cpuinfo file "
                  << std::system_category().message(errno) << std::endl;
    }

    ECpuInfo cpu;
    ECpuCore core;
    std::string line;

    // Regex pattern: matches things like "AMD Ryzen 5 7430U"
    const std::regex cpuPattern(R"(AMD\s+Ryzen\s+\d+\s+\d+\w*)", std::regex::icase);
    while (std::getline(cpuinfo, line))
    {
        if (line.starts_with("model name"))
        {
            std::string text = line.substr(line.find(":") + 2);
            std::smatch match;

            if (std::regex_search(text, match, cpuPattern))
            {
                std::cerr << "CPU Detected: " << match.str() << std::endl;
            }
            else
            {
                std::cerr << "CPU not found in text." << std::endl;
            }

            cpu.modelName = match.str();
        }

        else if (line.starts_with("cpu cores"))
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

        else if (line.starts_with("processor"))
        {
            core.name = line.substr(line.find(":") + 2);
        }

        else if (line.starts_with("cache size"))
        {
            core.cacheSize = line.substr(line.find(":") + 2);
        }

        else if (line.starts_with("cpu MHz"))
        {
            core.frequencyMhz = line.substr(line.find(":") + 2);
        }

        else if (line.starts_with("power management"))
        {
            core.usagePercent = readCpuUsagePercent();
            cpu.threads.push_back(core);
            core = {};
        }
    }

    info.cpu = cpu;
}

void ADLinuxSystemInfoReader::readCpuTempFromSys()
{
    namespace fs = std::filesystem;
    const fs::path thermalDir = "/sys/class/hwmon/hwmon1/temp1_input";

    std::ifstream tempFile(thermalDir);
    if (!tempFile.is_open()) {
        std::cerr << "Failed to open temperature file: " << thermalDir<< ")" << '\n';
        return;
    }

    std::string line;
    if (std::getline(tempFile, line)) {
        try {
            if (!info.cpu.threads.empty() )
            {
                info.cpu.threads.front().temperatureC = std::stoull(line);
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse temperature: " << e.what() << '\n';
        }
    }
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

std::string ADLinuxSystemInfoReader::readCpuUsagePercent() {
    CpuTimes t1 = readCpuTimes();
    std::this_thread::sleep_for(std::chrono::nanoseconds(6000000));
    CpuTimes t2 = readCpuTimes();

    unsigned long long idleDiff = t2.idleTime() - t1.idleTime();
    unsigned long long totalDiff = t2.total() - t1.total();

    if (totalDiff == 0) return "";

    double usage = 100.0 * (totalDiff - idleDiff) / totalDiff;
    char buf[10];
    snprintf(buf, sizeof(buf), "%.1f", usage);
    return std::string(buf);
}

void ADLinuxSystemInfoReader::readMemoryInfo() {
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    uint64_t total_bytes = 0;
    uint64_t availableKb = 0;

    while (std::getline(meminfo, line))
    {
        if (line.starts_with("MemTotal:"))
        {
            total_bytes = std::stoull(line.substr(9));
        } else if (line.starts_with("MemAvailable:"))
        {
            availableKb = std::stoull(line.substr(13));
        }

        if (total_bytes > 0 && availableKb > 0) break;
    }

    EMemoryInfo mem;
    mem.total_bytes = total_bytes / 1024;
    uint64_t availableMb = availableKb / 1024;
    mem.used_bytes = mem.total_bytes - availableMb;

    double percent = 100.0 * mem.used_bytes / mem.total_bytes;
    char buf[10];
    snprintf(buf, sizeof(buf), "%.1f", percent);
    mem.usage_percent = std::string(buf);

    info.mem = mem;
}

void ADLinuxSystemInfoReader::readGpuInfo()
{
    if (auto value = readLine("/sys/class/drm/card1/device/uevent"))
    {
        if (value->starts_with("DRIVER="))
        {
            info.gpu.name =  value->substr(value->find("=") + 1);
        }
    }

    if (auto value = readLine("/sys/class/drm/card1/device/mem_info_vram_total"))
    {
        info.gpu.vramTotal = (std::stoull(*value) / 1024) / 1024;
    }

    if (auto value = readLine("/sys/class/drm/card1/device/mem_info_vram_used"))
    {
        info.gpu.vramUsed = (std::stoull(*value) / 1024) / 1024;
    }
}

void ADLinuxSystemInfoReader::readNetworkInfo()
{
    uint64_t rxByte = 0;
    uint64_t txByte = 0;
    if (auto value = readLine("/sys/class/net/wlp1s0/statistics/rx_bytes"))
    {
        rxByte = std::stoull(*value);
    }

    if (auto value = readLine("/sys/class/net/wlp1s0/statistics/tx_bytes"))
    {
        txByte = std::stoull(*value);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (auto value = readLine("/sys/class/net/wlp1s0/statistics/rx_bytes"))
    {
        info.net.rxBytes = (std::stoull(*value) - rxByte) / 1024;
    }

    if (auto value = readLine("/sys/class/net/wlp1s0/statistics/tx_bytes"))
    {
        info.net.txBytes = (std::stoull(*value) - txByte) / 1024;
    }
}

std::optional<std::string> ADLinuxSystemInfoReader::readLine(const char* path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return std::nullopt;
    }

    std::string line;
    if (std::getline(file, line) && !line.empty())
    {
        return line;
    }

    return std::nullopt;
}

