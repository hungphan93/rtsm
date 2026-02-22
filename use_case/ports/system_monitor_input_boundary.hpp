#ifndef SYSTEM_MONITOR_INPUT_BOUNDARY_HPP
#define SYSTEM_MONITOR_INPUT_BOUNDARY_HPP
#include "entity/cpu.hpp"
#include "entity/memory.hpp"
#include "entity/disk.hpp"
#include "entity/net.hpp"
#include "entity/gpu.hpp"

namespace usecase {

struct system_monitor_input_boundary {

    virtual ~system_monitor_input_boundary() = default;

    virtual void on_cpu_updated(const entity::cpu& cpu) = 0;
    virtual void on_memory_updated(const entity::memory& memory) = 0;
    virtual void on_gpu_updated(const entity::gpu& gpu) = 0;
    virtual void on_disk_updated(const entity::disk& disk) = 0;
    virtual void on_net_updated(const entity::net& net) = 0;
};

} /// namespace usecase

#endif /// SYSTEM_MONITOR_INPUT_BOUNDARY_HPP
