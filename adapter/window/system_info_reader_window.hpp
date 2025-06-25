#ifndef ADAPTER_WINDOW_SYSTEM_INFO_READER_WINDOW_HPP
#define ADAPTER_WINDOW_SYSTEM_INFO_READER_WINDOW_HPP
#include "use_case/system_info_reader.hpp"

namespace adapter {
namespace window {

class system_info_reader_window : public usecase::system_info_reader {

public:
    entity::cpu read_cpu() const override;

    entity::memory read_memory() const override;

    entity::gpu read_gpu() const override;

    entity::disk read_disk() const override;

    entity::net read_net() const override;
};

} // namespace window
} // namespace adapter

#endif // ADAPTER_WINDOW_SYSTEM_INFO_READER_WINDOW_HPP
