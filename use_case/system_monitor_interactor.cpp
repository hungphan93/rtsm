#include "system_monitor_interactor.hpp"

namespace usecase {

system_monitor_interactor::system_monitor_interactor(system_monitor_output_boundary& output) noexcept : output_{output} {

}

system_monitor_interactor::~system_monitor_interactor() noexcept {

}

void system_monitor_interactor::on_cpu_updated(const entity::cpu& cpu) {
    output_.present_cpu(cpu);
}

void system_monitor_interactor::on_memory_updated(const entity::memory& memory) {
    output_.present_memory(memory);
}

void system_monitor_interactor::on_gpu_updated(const entity::gpu& gpu) {
    output_.present_gpu(gpu);
}

void system_monitor_interactor::on_disk_updated(const entity::disk& disk) {
    output_.present_disk(disk);
}

void system_monitor_interactor::on_net_updated(const entity::net& net) {
    output_.present_net(net);
}

}

