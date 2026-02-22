#ifndef SYSTEM_MONITOR_OUTPUT_BOUNDARY_HPP
#define SYSTEM_MONITOR_OUTPUT_BOUNDARY_HPP
#include "entity/cpu.hpp"
#include "entity/memory.hpp"
#include "entity/disk.hpp"
#include "entity/net.hpp"
#include "entity/gpu.hpp"

namespace usecase {

struct system_monitor_output_boundary {

    virtual ~system_monitor_output_boundary() = default;

    virtual void present_cpu(const entity::cpu& cpu) = 0;
    virtual void present_memory(const entity::memory& memory) = 0;
    virtual void present_gpu(const entity::gpu& gpu) = 0;
    virtual void present_disk(const entity::disk& disk) = 0;
    virtual void present_net(const entity::net& net) = 0;
};

} /// namespace usecase

#endif /// SYSTEM_MONITOR_OUTPUT_BOUNDARY_HPP
