#ifndef EMEMORYINFO_H
#define EMEMORYINFO_H

#include <cstdint>
#include <string>

struct EMemoryInfo
{
    uint64_t vramTotal = 0;
    uint64_t vramUsed = 0;
    uint64_t total_bytes = 0;
    uint64_t used_bytes = 0;
    std::string usage_percent;
    std::string name;
    double voltage = 0;
    double buss = 0;
    double swap = 0;
};

#endif // EMEMORYINFO_H
