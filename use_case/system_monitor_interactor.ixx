/// MIT License
module;

#include <mutex>

export module usecase:system_monitor_interactor;

import :system_monitor_input_boundary;
import :system_monitor_output_boundary;
import :system_info_reader;

export namespace usecase {

class system_monitor_interactor : public system_monitor_input_boundary {

public:
    explicit system_monitor_interactor(const system_info_reader& reader, system_monitor_output_boundary& output) noexcept
        : reader_{reader}, output_{output} {}

    ~system_monitor_interactor() noexcept override = default;

    void fetch_cpu() override {
        std::scoped_lock lock(cpu_mtx_);
        output_.present_cpu(reader_.read_cpu());
    }

    void fetch_memory() override {
        std::scoped_lock lock(memory_mtx_);
        output_.present_memory(reader_.read_memory());
    }

    void fetch_gpu() override {
        std::scoped_lock lock(gpu_mtx_);
        output_.present_gpu(reader_.read_gpu());
    }

    void fetch_disk() override {
        std::scoped_lock lock(disk_mtx_);
        output_.present_disk(reader_.read_disk());
    }

    void fetch_net() override {
        std::scoped_lock lock(net_mtx_);
        output_.present_net(reader_.read_net());
    }

private:
    const system_info_reader& reader_;
    system_monitor_output_boundary& output_;

    /// Per-metric mutexes to serialize concurrent fetch calls (e.g. subscribe + run_once)
    std::mutex cpu_mtx_;
    std::mutex memory_mtx_;
    std::mutex gpu_mtx_;
    std::mutex disk_mtx_;
    std::mutex net_mtx_;
};

} /// namespace usecase
