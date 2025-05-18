#ifndef EDISKINFO_H
#define EDISKINFO_H
#include <string>
#include <cstdint>

struct EDiskInfo
{
    uint64_t readSpeed = 0;
    uint64_t writeSpeed = 0;
    uint64_t sectorSize = 0;
    std::string model;
};

#endif // EDISKINFO_H
