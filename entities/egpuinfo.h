#ifndef EGPUINFO_H
#define EGPUINFO_H

#include <cstdint>
#include <string>

struct EGpuInfo
{
    std::string name;
    uint64_t vramTotal;
    uint64_t vramUsed;
    double usagePercent;
    uint64_t cores;
    double frequencyMhz;
    double temperatureC;
    double power;
};
#endif // EGPUINFO_H
