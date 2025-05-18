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
#include <vector>
#include <thread>
#include <chrono>
#include <regex>

ADLinuxSystemInfoReader::ADLinuxSystemInfoReader(): info{}
{

}
ADLinuxSystemInfoReader::~ADLinuxSystemInfoReader()
{
}
ESystemInfo ADLinuxSystemInfoReader::read()
{
    std::cerr << "log ADLinuxSystemInfoReader::read\n";
    readCpuInfoFromProc();
    readNetworkInfo();
    readDiskInfo();
    readCpuTempFromSys();
    readMemoryInfo();
    readGpuInfo();
    return info;
}

std::optional<int> ADLinuxSystemInfoReader::parseInt(const std::string& s) {
    std::cerr << "log ADLinuxSystemInfoReader::parseInt\n";
    try {
        return std::stoi(s);
    } catch (...) {
        std::cerr  << "Failed to parse number" <<std::endl;
        return std::nullopt;
    }
}

void ADLinuxSystemInfoReader::readCpuInfoFromProc()
{
    std::cerr << "log ADLinuxSystemInfoReader::readCpuInfoFromProc\n";
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
                cpu.modelName = match.str();
            }
            else
            {
                std::cerr << "CPU not found in text." << std::endl;
                cpu.modelName = "Unknown";
            }
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
            core = ECpuCore{};
        }
    }

    info.cpu = cpu;
}

void ADLinuxSystemInfoReader::readCpuTempFromSys()
{
    std::cerr << "log ADLinuxSystemInfoReader::readCpuTempFromSys\n";
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
    std::cerr << "log ADLinuxSystemInfoReader::readCpuTimes\n";
    std::ifstream file("/proc/stat");
    std::string line;
    std::getline(file, line);

    std::istringstream iss(line);
    std::string cpu_label;
    CpuTimes times;
    iss >> cpu_label >> times.user >> times.nice >> times.system >> times.idle
        >> times.iowait >> times.irq >> times.softirq >> times.steal;

    std::cerr << "log ADLinuxSystemInfoReader::readCpuTimes2\n";
    return times;
}

std::string ADLinuxSystemInfoReader::readCpuUsagePercent() {
    std::cerr << "log ADLinuxSystemInfoReader::readCpuUsagePercent\n";
    CpuTimes t1 = readCpuTimes();
    std::this_thread::sleep_for(std::chrono::nanoseconds(19000000));
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
    std::cerr << "log ADLinuxSystemInfoReader::readMemoryInfo\n";
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
    std::cerr << "log ADLinuxSystemInfoReader::readGpuInfo\n";
    std::string value = readLine("/sys/class/drm/card1/device/uevent");
    if (!value.empty())
    {
        size_t pos = value.find("DRIVER=");
        if (pos != std::string::npos)
            info.gpu.name = value.substr(pos + 1);

        // if (value.starts_with("DRIVER="))
        // {
        //     info.gpu.name =  value.substr(value.find("=") + 1);
        // }
    }

    value = readLine("/sys/class/drm/card1/device/mem_info_vram_total");
    if (!value.empty())
    {
        info.gpu.vramTotal = (std::stoull(value) / 1024) / 1024;
    }

    value = readLine("/sys/class/drm/card1/device/mem_info_vram_used");
    if (!value.empty())
    {
        info.gpu.vramUsed = (std::stoull(value) / 1024) / 1024;
    }
}

void ADLinuxSystemInfoReader::readNetworkInfo()
{
    std::cerr << "log ADLinuxSystemInfoReader::readNetworkInfo\n";
    uint64_t rxByte = 0;
    uint64_t txByte = 0;

    std::string value = readLine("/sys/class/net/wlp1s0/statistics/rx_bytes");
    if (!value.empty())
    {
        rxByte = std::stoull(value);
    }

    value = readLine("/sys/class/net/wlp1s0/statistics/tx_bytes");
    std::cerr << "log ADLinuxSystemInfoReader::readNetworkInfo2 - "<< value << " \n";
    if (!value.empty())
    {
        txByte = std::stoull(value);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cerr << "log ADLinuxSystemInfoReader::readNetworkInfo555 - " << value << " \n";
    value = readLine("/sys/class/net/wlp1s0/statistics/rx_bytes");
    std::cerr << "log ADLinuxSystemInfoReader::readNetworkInfo3\n";
    if (!value.empty())
    {
        info.net.rxBytes = (std::stoull(value) - rxByte) / 1024;
    }

    value = readLine("/sys/class/net/wlp1s0/statistics/tx_bytes");
    if (!value.empty())
    {
        info.net.txBytes = (std::stoull(value) - txByte) / 1024;
    }
    std::cerr << "log ADLinuxSystemInfoReader::readNetworkInfo4\n";
}

void ADLinuxSystemInfoReader::readDiskInfo()
{
    std::cerr << "log ADLinuxSystemInfoReader::readDiskInfo\n";
    std::string device = "nvme0n1";
    std::string str = "/sys/block/" + device + "/queue/hw_sector_size";
    std::string value = readLine(str.c_str());
    if (value.empty()) return;
    info.disk.sectorSize = std::stoull(value);

    std::string str2 = "/sys/block/" + device + "/device/model";
    std::string value2 = readLine(str2.c_str());
    if (value2.empty()) return;
    info.disk.model = value2;

    auto fetchDiskStats = [&device]() -> std::tuple<uint64_t, uint64_t>
    {
        std::ifstream diskInfo("/proc/diskstats");
        if (!diskInfo.is_open()) return {0,0};

        std::string line;
        std::vector<std::string> tokens;
        while (std::getline(diskInfo, line))
        {
            //find first line nvme0n1 string then push word into tokens vector
            if (line.find(device) != std::string::npos)
            {
                std::istringstream iss(line);

                std::string token;
                while (iss >> token)
                    tokens.push_back(token);

                if (tokens.size() > 9)
                    return {std::stoull(tokens[5]),std::stoull(tokens[9])};

                break;
            }
        }

        return {0,0};
    };

    auto [r1, w1] = fetchDiskStats();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto [r2, w2] = fetchDiskStats();
    //cal read && write speed of disk Mb/s
    info.disk.readSpeed = (r2 - r1) * info.disk.sectorSize / 1024.0 / 1024.0;
    info.disk.writeSpeed = (w2 - w1) * info.disk.sectorSize / 1024.0 / 1024.0;
}

std::string ADLinuxSystemInfoReader::readLine(const char* path)
{
    std::ifstream file(path);
    std::string line;

    if (file.is_open() && std::getline(file, line))
    {
        std::cerr << "log ADLinuxSystemInfoReader::readLine - "<< line << "\n";
        return line;
    }

    std::cerr << "log ADLinuxSystemInfoReader::readLine2- " << line << " \n";
    return "";
}
