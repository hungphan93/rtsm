/// MIT License
#ifndef SYSTEM_MONITOR_INTERACTOR_HPP
#define SYSTEM_MONITOR_INTERACTOR_HPP
#include "ports/system_monitor_input_boundary.hpp"
#include "ports/system_monitor_output_boundary.hpp"
#include "ports/system_info_reader.hpp"

namespace usecase {

class system_monitor_interactor : public system_monitor_input_boundary {

public:
    explicit system_monitor_interactor(const system_info_reader& reader, system_monitor_output_boundary& output) noexcept;

    ~system_monitor_interactor() noexcept override = default;

    void fetch_cpu() override;
    void fetch_memory() override;
    void fetch_gpu() override;
    void fetch_disk() override;
    void fetch_net() override;

private:
    const system_info_reader& reader_;
    system_monitor_output_boundary& output_;
};

}

#endif /// SYSTEM_MONITOR_INTERACTOR_HPP
