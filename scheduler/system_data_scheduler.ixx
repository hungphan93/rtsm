/// MIT License
export module system_data_scheduler;

import std;

export namespace scheduler {

class system_data_scheduler {
public:
    using subscription_id = std::size_t;

    explicit system_data_scheduler() noexcept = default;
    ~system_data_scheduler() noexcept;

    system_data_scheduler(const system_data_scheduler&) = delete;
    system_data_scheduler& operator=(const system_data_scheduler&) = delete;
    system_data_scheduler(system_data_scheduler&&) = delete;
    system_data_scheduler& operator=(system_data_scheduler&&) = delete;


    [[nodiscard]] subscription_id subscribe(
        std::chrono::milliseconds interval,
        std::function<void()> task) {
        const auto id = next_id_.fetch_add(1);
        std::scoped_lock lock(mutex_);
        subscriptions_.emplace(id, std::jthread(
                                       [this, interval, cb = std::move(task)](std::stop_token st) {
                                           std::mutex sleep_mutex;
                                           std::condition_variable_any cv;
                                           while (!st.stop_requested()) {
                                               cb();
                                               // Interruptible sleep: 
                                               // This wakes up IMMEDIATELY when the stop token is triggered by IDE/App Exit
                                               std::unique_lock<std::mutex> lock(sleep_mutex);
                                               cv.wait_for(lock, st, interval, []{ return false; });
                                           }
                                       }));
        return id;
    }

    // Runs a task asynchronously to get delta-accurate data (e.g. CPU requires 2 readings over an interval)
    // The thread is fully managed and safely self-cleans without deadlinking the map lock.
    [[nodiscard]] subscription_id run_once(std::chrono::milliseconds interval, std::function<void()> task) {
        const auto id = next_id_.fetch_add(1);
        std::scoped_lock lock(mutex_);
        subscriptions_.emplace(id, std::jthread(
                                       [this, id, interval, cb = std::move(task)](std::stop_token st) {
                                           std::mutex sleep_mutex;
                                           std::condition_variable_any cv;

                                           // Baseline Call
                                           if (!st.stop_requested()) cb(); 

                                           if (interval > std::chrono::milliseconds::zero()) {
                                               std::unique_lock<std::mutex> lock(sleep_mutex);
                                               cv.wait_for(lock, st, interval, []{ return false; });
                                           }

                                           // Delta Call
                                           if (!st.stop_requested()) cb();

                                           // Safely self-clean the subscription (using a detached secondary thread to prevent erase() deadlocks on the dying primary thread)
                                           std::thread([this, id]() { this->unsubscribe(id); }).detach(); 
                                       }));
        return id;
    }

    void unsubscribe(subscription_id id);
    void stop_all();
    [[nodiscard]] std::size_t active_count() const noexcept;

private:
    std::map<subscription_id, std::jthread> subscriptions_;
    std::atomic<subscription_id> next_id_{0};
    mutable std::mutex mutex_;
};

} /// namespace scheduler
