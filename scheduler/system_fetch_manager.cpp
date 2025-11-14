/// MIT License
#include "system_fetch_manager.hpp"

namespace scheduler {

system_fetch_manager::~system_fetch_manager() noexcept {
    stop_all();
}

void system_fetch_manager::stop_all() noexcept {
    fetchers_.clear();
}

void system_fetch_manager::add_fetcher(const std::string &name,
                                       std::chrono::milliseconds interval,
                                       callback cb) noexcept {
    if (fetchers_.find(name) != fetchers_.end()) {
        return;
    }

    fetchers_.emplace(name, std::jthread([interval, cb] (std::stop_token st) {
        while (!st.stop_requested()) {
            cb();
            std::this_thread::sleep_for(interval);
        }
     }));
}

} // namespace scheduler
