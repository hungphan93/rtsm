#ifndef ADLINUXSYSTEMINFOREADER_H
#define ADLINUXSYSTEMINFOREADER_H

#include "entities/esysteminfo.h"
#include <string>
#include <QString>
#include <optional>
#include <expected>
#include <interfaces/isysteminforeader.h>

struct ESystemInfo;

struct CpuTimes {
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;

    unsigned long long total() const
    {
        return user + nice + system + idle + iowait + irq + softirq + steal;
    }

    unsigned long long idleTime() const
    {
        return idle + iowait;
    }
};

class ADLinuxSystemInfoReader: public ISystemInfoReader
{
public:
    explicit ADLinuxSystemInfoReader() = default;
    virtual ESystemInfo &read() override;

private:
    ESystemInfo info;
    std::optional<int> parseInt(const std::string &s);
    void readCpuInfoFromProc();
    void readCpuTempFromSys();
    CpuTimes readCpuTimes();
    std::string readCpuUsagePercent();
    void readMemoryInfo();
    void readGpuInfo();
};

#endif // ADLINUXSYSTEMINFOREADER_H
