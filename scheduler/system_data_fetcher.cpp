/// MIT License
#include "system_data_fetcher.hpp"

namespace scheduler {

system_data_fetcher::system_data_fetcher(const usecase::system_info_reader& reader) noexcept
    : reader_(reader) {}

entity::cpu system_data_fetcher::fetch_cpu() const {
    return reader_.read_cpu();
}

entity::memory system_data_fetcher::fetch_memory() const {
    return reader_.read_memory();
}

entity::gpu system_data_fetcher::fetch_gpu() const {
    return reader_.read_gpu();
}

entity::disk system_data_fetcher::fetch_disk() const {
    return reader_.read_disk();
}

entity::net system_data_fetcher::fetch_net() const {
    return reader_.read_net();
}

} /// namespace scheduler
