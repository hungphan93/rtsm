/// MIT License
#ifndef ADAPTER_LINUX_SYSTEM_INFO_READER_LINUX_HPP
#define ADAPTER_LINUX_SYSTEM_INFO_READER_LINUX_HPP

#include "use_case/system_info_reader.hpp"

namespace adapter {
namespace linux2 {

struct system_info_reader_linux : public usecase::system_info_reader {

    explicit system_info_reader_linux() noexcept;

    entity::cpu read_cpu() const override;

    entity::memory read_memory() const override;

    entity::gpu read_gpu() const override;

    entity::disk read_disk() const override;

    entity::net read_net() const override;
};

} /// namespace linux2
} /// namespace adapter

#endif /// ADAPTER_LINUX_SYSTEM_INFO_READER_LINUX_HPP
