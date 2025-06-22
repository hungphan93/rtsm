#ifndef ADLINUXSYSTEMINFOREADER_H
#define ADLINUXSYSTEMINFOREADER_H

#include "entities/esysteminfo.h"
#include <string>
#include <optional>
#include <mutex>
#include <interfaces/isysteminforeader.h>

struct ESystemInfo;

struct CpuTimes
{
    unsigned long long user = 0;
    unsigned long long nice = 0;
    unsigned long long system = 0;
    unsigned long long idle = 0;
    unsigned long long iowait = 0;
    unsigned long long irq = 0;
    unsigned long long softirq = 0;
    unsigned long long steal = 0;

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
    explicit ADLinuxSystemInfoReader();
    ~ADLinuxSystemInfoReader();
    virtual ESystemInfo read() override;

private:
    ESystemInfo info;
    std::optional<int> parseInt(const std::string &s);
    void readCpuInfoFromProc();
    void readCpuTempFromSys();
    CpuTimes readCpuTimes();
    std::string readCpuUsagePercent();
    void readMemoryInfo();
    void readGpuInfo();
    void readNetworkInfo();
    void readDiskInfo();
    std::string readLine(const char* path);
    std::mutex m_mutex;
};

#endif // ADLINUXSYSTEMINFOREADER_H
