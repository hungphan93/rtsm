#ifndef ISYSTEMINFOREADER_H
#define ISYSTEMINFOREADER_H

struct ESystemInfo;

struct ISystemInfoReader
{
    virtual ~ISystemInfoReader() = default;
    virtual  ESystemInfo read() = 0;
};

#endif // ISYSTEMINFOREADER_H
