#include "ucsysteminforeader.h"
#include "entities/esysteminfo.h"
#include "interfaces/isysteminforeader.h"

UCSystemInfoReader::UCSystemInfoReader(ISystemInfoReader& reader): reader{reader}
{

}

ESystemInfo UCSystemInfoReader::execute() const
{
    return reader.read();
}
