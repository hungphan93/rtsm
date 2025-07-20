#ifndef PRESENTER_SYSTEM_MONITOR_HPP
#define PRESENTER_SYSTEM_MONITOR_HPP
#include "scheduler/system_fetch_manager.hpp"
#include "use_case/system_info_reader.hpp"
#include <mutex>

namespace presenter {

class system_monitor {

public:
    explicit system_monitor(const usecase::system_info_reader& reader) noexcept;
    ~system_monitor() noexcept;

    [[nodiscard]] entity::cpu cpu() const noexcept;
    [[nodiscard]] entity::memory memory() const noexcept;
    [[nodiscard]] entity::gpu gpu() const noexcept;
    [[nodiscard]] entity::disk disk() const noexcept;
    [[nodiscard]] entity::net net() const noexcept;

private:
    void start_periodic_tasks() noexcept;
    const usecase::system_info_reader& reader_;
    scheduler::system_fetch_manager scheduler_;
    mutable std::mutex mutex_;
    entity::cpu last_cpu_;
    entity::memory last_memory_;
    entity::disk last_disk_;
    entity::gpu last_gpu_;
    entity::net last_net_;
};

} // namespace presenter

#endif // PRESENTER_SYSTEM_MONITOR_HPP
