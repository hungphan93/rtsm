#ifndef EGPUINFO_H
#define EGPUINFO_H

#include <cstdint>
#include <string>

struct EGpuInfo
{
    std::string name;
    uint64_t vramTotal = 0;
    uint64_t vramUsed = 0;
    uint64_t usagePercent = 0;
    uint64_t cores = 0;
    uint64_t frequencyMhz = 0;
    uint64_t temperatureC = 0;
    uint64_t power = 0;
};
#endif // EGPUINFO_H
