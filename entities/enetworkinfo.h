#ifndef ENETWORKINFO_H
#define ENETWORKINFO_H

#include <cstdint>

struct ENetworkInfo
{
    uint64_t rxBytes = 0;
    uint64_t txBytes = 0;
};

#endif // ENETWORKINFO_H
