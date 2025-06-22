#ifndef USECASE_SYSTEM_INFO_READER_HPP
#define USECASE_SYSTEM_INFO_READER_HPP
#include "entity/cpu.hpp"
#include "entity/disk.hpp"

namespace usecase {

struct system_info_reader
{
    virtual ~system_info_reader() = default;
    virtual entity::cpu read_cpu() = 0;
    virtual entity::disk read_disk() = 0;

};

} // namespace useca

#endif // USECASE_SYSTEM_INFO_READER_HPP
