/// MIT License
#ifndef SYSTEM_MONITOR_VIEW_MODEL_HPP
#define SYSTEM_MONITOR_VIEW_MODEL_HPP

#include "use_case/ports/system_monitor_output_boundary.hpp"
#include <functional>
#include <mutex>

namespace presenter {

/// This class is MODEL in MVC pattern
class system_monitor_view_model : public usecase::system_monitor_output_boundary {
public:
    using notify = std::function<void()>;

    system_monitor_view_model() noexcept = default;
    ~system_monitor_view_model() noexcept override = default;

    system_monitor_view_model(const system_monitor_view_model&) = delete;
    system_monitor_view_model& operator=(const system_monitor_view_model&) = delete;

    /// MODEL
    [[nodiscard]] entity::cpu cpu() const noexcept       { return get(cpu_); }
    [[nodiscard]] entity::memory memory() const noexcept { return get(memory_); }
    [[nodiscard]] entity::gpu gpu() const noexcept       { return get(gpu_); }
    [[nodiscard]] entity::disk disk() const noexcept     { return get(disk_); }
    [[nodiscard]] entity::net net() const noexcept       { return get(net_); }

    /// Subscribe
    void on_cpu_changed(notify cb)    { cpu_.cb = std::move(cb); }
    void on_memory_changed(notify cb) { memory_.cb = std::move(cb); }
    void on_gpu_changed(notify cb)    { gpu_.cb = std::move(cb); }
    void on_disk_changed(notify cb)   { disk_.cb = std::move(cb); }
    void on_net_changed(notify cb)    { net_.cb = std::move(cb); }

private:
    template<typename T>
    struct field {
        T data{};
        notify cb;
    };

    template<typename T>
    [[nodiscard]] T get(const field<T>& f) const noexcept {
        std::scoped_lock lock(mutex_);
        return f.data;
    }

    template<typename T>
    void update(field<T>& f, const T& v) {
        { std::scoped_lock lock(mutex_); f.data = v; }
        if (f.cb) f.cb();
    }

    void present_cpu(const entity::cpu& v) override       { update(cpu_, v); }
    void present_memory(const entity::memory& v) override { update(memory_, v); }
    void present_gpu(const entity::gpu& v) override       { update(gpu_, v); }
    void present_disk(const entity::disk& v) override     { update(disk_, v); }
    void present_net(const entity::net& v) override       { update(net_, v); }

    mutable std::mutex mutex_;
    field<entity::cpu> cpu_;
    field<entity::memory> memory_;
    field<entity::gpu> gpu_;
    field<entity::disk> disk_;
    field<entity::net> net_;
};

} /// namespace presenter

#endif /// SYSTEM_MONITOR_VIEW_MODEL_HPP
