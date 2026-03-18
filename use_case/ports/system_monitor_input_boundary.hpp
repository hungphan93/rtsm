#ifndef SYSTEM_MONITOR_INPUT_BOUNDARY_HPP
#define SYSTEM_MONITOR_INPUT_BOUNDARY_HPP

namespace usecase {

struct system_monitor_input_boundary {

    virtual ~system_monitor_input_boundary() = default;

    virtual void fetch_cpu() = 0;
    virtual void fetch_memory() = 0;
    virtual void fetch_gpu() = 0;
    virtual void fetch_disk() = 0;
    virtual void fetch_net() = 0;
};

} /// namespace usecase

#endif /// SYSTEM_MONITOR_INPUT_BOUNDARY_HPP
