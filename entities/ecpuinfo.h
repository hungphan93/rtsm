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
    std::string temperatureC;
    std::string cacheSize;
};

struct ECpuInfo
{
    uint64_t cache;
    uint64_t coreNumber;
    double power;
    std::string modelName;
    std::vector<ECpuCore> threads;
};


#endif // ECPUINFO_H
