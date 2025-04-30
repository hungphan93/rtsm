#ifndef EMEMORYINFO_H
#define EMEMORYINFO_H

#include <cstdint>
#include <string>

struct EMemoryInfo
{
    uint64_t vramTotal;
    uint64_t vramUsed;
    uint64_t total_bytes;
    uint64_t used_bytes;
    std::string usage_percent;
    std::string name;
    double voltage;
    double buss;
    double swap;
};

#endif // EMEMORYINFO_H
