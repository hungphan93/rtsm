#include "system_monitor_interactor.hpp"

namespace usecase {

system_monitor_interactor::system_monitor_interactor(const system_info_reader& reader, system_monitor_output_boundary& output) noexcept : reader_{reader}, output_{output} {

}

void system_monitor_interactor::fetch_cpu() {
    output_.present_cpu(reader_.read_cpu());
}

void system_monitor_interactor::fetch_memory() {
    output_.present_memory(reader_.read_memory());
}

void system_monitor_interactor::fetch_gpu() {
    output_.present_gpu(reader_.read_gpu());
}

void system_monitor_interactor::fetch_disk() {
    output_.present_disk(reader_.read_disk());
}

void system_monitor_interactor::fetch_net() {
    output_.present_net(reader_.read_net());
}

}

