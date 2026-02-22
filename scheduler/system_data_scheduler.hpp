/// MIT License
#ifndef SYSTEM_DATA_SCHEDULER_HPP
#define SYSTEM_DATA_SCHEDULER_HPP

#include "../use_case/ports/system_info_reader.hpp"
#include <chrono>
#include <thread>
#include <map>
#include <mutex>
#include <atomic>

namespace scheduler {

class system_data_scheduler {
public:
    using subscription_id = std::size_t;

    explicit system_data_scheduler(const usecase::system_info_reader& reader) noexcept;
    ~system_data_scheduler() noexcept;

    system_data_scheduler(const system_data_scheduler&) = delete;
    system_data_scheduler& operator=(const system_data_scheduler&) = delete;
    system_data_scheduler(system_data_scheduler&&) = delete;
    system_data_scheduler& operator=(system_data_scheduler&&) = delete;


    template<typename T, typename Fn>
    [[nodiscard]] subscription_id subscribe(
        std::chrono::milliseconds interval,
        T(usecase::system_info_reader::*reader_fn)() const,
        Fn&& callback) {
        const auto id = next_id_.fetch_add(1);
        std::scoped_lock lock(mutex_);
        subscriptions_.emplace(id, std::jthread(
                                       [this, interval, reader_fn, cb = std::forward<Fn>(callback)](std::stop_token st) {
                                           while (!st.stop_requested()) {
                                               cb((reader_.*reader_fn)());
                                               std::this_thread::sleep_for(interval);
                                           }
                                       }));
        return id;
    }

    void unsubscribe(subscription_id id);
    void stop_all();
    [[nodiscard]] std::size_t active_count() const noexcept;

private:
    const usecase::system_info_reader& reader_;
    std::map<subscription_id, std::jthread> subscriptions_;
    std::atomic<subscription_id> next_id_{0};
    mutable std::mutex mutex_;
};

} /// namespace scheduler

#endif /// SYSTEM_DATA_SCHEDULER_HPP
