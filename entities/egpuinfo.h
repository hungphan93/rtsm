#ifndef EGPUINFO_H
#define EGPUINFO_H

#include <cstdint>
#include <string>

struct EGpuInfo
{
    std::string name;
    uint64_t vramTotal;
    uint64_t vramUsed;
    uint64_t usagePercent;
    uint64_t cores;
    uint64_t frequencyMhz;
    uint64_t temperatureC;
    uint64_t power;
};
#endif // EGPUINFO_H
