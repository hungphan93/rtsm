#include "system_monitor.hpp"

namespace presenter {

system_monitor::system_monitor(const usecase::system_info_reader& reader) noexcept : reader_(reader) {
}

entity::cpu system_monitor::cpu() const {
    return reader_.read_cpu();
}

entity::memory system_monitor::memory() const {
    return reader_.read_memory();
}

entity::gpu system_monitor::gpu() const {
    return reader_.read_gpu();
}

entity::disk system_monitor::disk() const {
    return reader_.read_disk();
}

entity::net system_monitor::net() const {
    return reader_.read_net();
}

} // namespace presenter
