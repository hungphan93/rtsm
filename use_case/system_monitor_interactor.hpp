/// MIT License
#ifndef SYSTEM_MONITOR_INTERACTOR_HPP
#define SYSTEM_MONITOR_INTERACTOR_HPP
#include "ports/system_monitor_input_boundary.hpp"
#include "ports/system_monitor_output_boundary.hpp"

namespace usecase {

class system_monitor_interactor : public system_monitor_input_boundary {

public:
    explicit system_monitor_interactor(system_monitor_output_boundary& output) noexcept;

    ~system_monitor_interactor() noexcept override = default;

    void on_cpu_updated(const entity::cpu& cpu) override;
    void on_memory_updated(const entity::memory& memory) override;
    void on_gpu_updated(const entity::gpu& gpu) override;
    void on_disk_updated(const entity::disk& disk) override;
    void on_net_updated(const entity::net& net) override;

private:
    system_monitor_output_boundary& output_;
};

}

#endif /// SYSTEM_MONITOR_INTERACTOR_HPP
