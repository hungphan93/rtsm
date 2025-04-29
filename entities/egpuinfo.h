#ifndef EGPUINFO_H
#define EGPUINFO_H

#include <cstdint>
#include <string>

struct EGpuInfo
{
    std::string name;
    uint64_t total_bytes;
    uint64_t used_bytes;
    double usage_percent;
    uint64_t cores;
    double frequencyMhz;
    double temperatureC;
    double power;
};
#endif // EGPUINFO_H
