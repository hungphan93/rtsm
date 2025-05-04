#ifndef ECPUINFO_H
#define ECPUINFO_H

#include <string>
#include <cstdint>
#include <vector>

struct ECpuCore
{
    std::string name;
    std::string usagePercent;
    std::string frequencyMhz;
    uint64_t temperatureC;
    std::string cacheSize;
};

struct ECpuInfo
{
    uint64_t cache;
    uint64_t coreNumber;
    uint64_t power;
    std::string modelName;
    std::vector<ECpuCore> threads;
};


#endif // ECPUINFO_H
