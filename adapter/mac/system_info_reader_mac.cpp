/// MIT License
#include "system_info_reader_mac.hpp"

namespace adapter {
namespace mac {

entity::cpu system_info_reader_mac::read_cpu() const {
    return {};
}

entity::gpu system_info_reader_mac::read_gpu() const {
    return {};
}

entity::memory system_info_reader_mac::read_memory() const {
    return {};
}

entity::net system_info_reader_mac::read_net() const {
    return {};
}

entity::disk system_info_reader_mac::read_disk() const {
    return {};
}

} // namespace mac
} // namespace adapter
