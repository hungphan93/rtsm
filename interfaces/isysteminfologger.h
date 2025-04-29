#ifndef ISYSTEMINFOLOGGER_H
#define ISYSTEMINFOLOGGER_H

struct ISystemInfoLogger
{
    virtual ~ISystemInfoLogger() = default;
    virtual void log() const = 0;
};

#endif // ISYSTEMINFOLOGGER_H
