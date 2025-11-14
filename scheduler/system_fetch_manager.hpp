/// MIT License
#ifndef SCHEDULER_SYSTEM_FETCH_MANAGER_HPP
#define SCHEDULER_SYSTEM_FETCH_MANAGER_HPP

#include <functional>
#include <thread>
#include <map>

namespace scheduler {

class system_fetch_manager {
public:
    using callback = std::function<void()>;

    system_fetch_manager() noexcept = default;
    ~system_fetch_manager() noexcept;

    void stop_all() noexcept;

    void add_fetcher(const std::string& name,
                     std::chrono::milliseconds interval,
                     callback cb) noexcept;

private:
    std::map<std::string, std::jthread> fetchers_;
};

} // namespace scheduler

#endif // SCHEDULER_SYSTEM_FETCH_MANAGER_HPP
