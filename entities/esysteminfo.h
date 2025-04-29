#ifndef ESYSTEMINFO_H
#define ESYSTEMINFO_H
#include "entities/ecpuinfo.h"
#include "entities/egpuinfo.h"
#include "entities/ememoryinfo.h"

struct ESystemInfo
{
    ECpuInfo cpu;
    EGpuInfo gpu;
    EMemoryInfo mem;
    virtual ~ESystemInfo() = default;
};

#endif // ESYSTEMINFO_H
