#ifndef ESYSTEMINFO_H
#define ESYSTEMINFO_H
#include "ecpuinfo.h"
#include "egpuinfo.h"
#include "ememoryinfo.h"
#include "enetworkinfo.h"

struct ESystemInfo
{
    ECpuInfo cpu;
    EGpuInfo gpu;
    EMemoryInfo mem;
    ENetworkInfo net;
    virtual ~ESystemInfo() = default;
};

#endif // ESYSTEMINFO_H
