#ifndef UCSYSTEMINFOREADER_H
#define UCSYSTEMINFOREADER_H

struct ESystemInfo;
struct ISystemInfoReader;

class UCSystemInfoReader
{
public:
    explicit UCSystemInfoReader(ISystemInfoReader& reader);
    ESystemInfo execute() const;
private:
    ISystemInfoReader& reader;
};

#endif // UCSYSTEMINFOREADER_H
