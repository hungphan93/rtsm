/// MIT License
#ifndef ADAPTER_MAC_SYSTEM_INFO_READER_MAC_HPP
#define ADAPTER_MAC_SYSTEM_INFO_READER_MAC_HPP

#include "use_case/ports/system_info_reader.hpp"

namespace adapter {
namespace mac {

class system_info_reader_mac : public usecase::system_info_reader {

public:
    entity::cpu read_cpu() const override;

    entity::memory read_memory() const override;

    entity::gpu read_gpu() const override;

    entity::disk read_disk() const override;

    entity::net read_net() const override;
};

} // namespace mac
} // namespace adapter

#endif // ADAPTER_MAC_SYSTEM_INFO_READER_MAC_HPP
