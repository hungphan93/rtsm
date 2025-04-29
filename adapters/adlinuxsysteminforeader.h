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
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;

    unsigned long long total() const {
        return user + nice + system + idle + iowait + irq + softirq + steal;
    }

    unsigned long long idleTime() const {
        return idle + iowait;
    }
};

class ADLinuxSystemInfoReader: public ISystemInfoReader
{
public:
    explicit ADLinuxSystemInfoReader();
    virtual ESystemInfo& read() override;

private:
    ESystemInfo info;
    std::optional<int> parseInt(const std::string& s);
    ECpuInfo readCpuInfoFromProc();
    std::string readCpuTempFromSys();
    CpuTimes readCpuTimes();
    std::string getCpuUsagePercent();
    EMemoryInfo readMemoryInfo();
};

#endif // ADLINUXSYSTEMINFOREADER_H
