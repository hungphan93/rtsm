#include "system_monitor.hpp"

namespace presenter {

system_monitor::system_monitor(const usecase::system_info_reader& reader) noexcept : reader_(reader) {
    start_periodic_tasks();
}

system_monitor::~system_monitor() noexcept {
    scheduler_.stop_all();
}

entity::cpu system_monitor::cpu() const noexcept {
    std::scoped_lock lock(mutex_);
    return last_cpu_;
}

entity::memory system_monitor::memory() const noexcept {
    std::scoped_lock lock(mutex_);
    return last_memory_;
}

entity::gpu system_monitor::gpu() const noexcept {
    std::scoped_lock lock(mutex_);
    return last_gpu_;
}

entity::disk system_monitor::disk() const noexcept {
    std::scoped_lock lock(mutex_);
    return last_disk_;
}

entity::net system_monitor::net() const noexcept {
    std::scoped_lock lock(mutex_);
    return last_net_;
}

void system_monitor::start_periodic_tasks() noexcept {
    using namespace std::chrono;
    scheduler_.add_fetcher("cpu", 300ms, [this] {
        auto data = reader_.read_cpu();
        std::scoped_lock lock(mutex_);
        last_cpu_ = std::move(data);
    });

    scheduler_.add_fetcher("memory", 500ms, [this] {
        auto data = reader_.read_memory();
        std::scoped_lock lock(mutex_);
        last_memory_ = std::move(data);
    });

    scheduler_.add_fetcher("gpu", 500ms, [this] {
        auto data = reader_.read_gpu();
        std::scoped_lock lock(mutex_);
        last_gpu_ = std::move(data);
    });

    scheduler_.add_fetcher("disk", 1s, [this] {
        auto data = reader_.read_disk();
        std::scoped_lock lock(mutex_);
        last_disk_ = std::move(data);
    });

    scheduler_.add_fetcher("net", 1s, [this] {
        auto data = reader_.read_net();
        std::scoped_lock lock(mutex_);
        last_net_ = std::move(data);
    });
}

} // namespace presenter
