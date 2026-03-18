/// MIT License
#include "system_data_scheduler.hpp"

namespace scheduler {

system_data_scheduler::~system_data_scheduler() noexcept {
    stop_all();
}

void system_data_scheduler::unsubscribe(subscription_id id) {
    std::scoped_lock lock(mutex_);

    auto it = subscriptions_.find(id);
    if (it != subscriptions_.end()) {
        subscriptions_.erase(it);
    }
}

void system_data_scheduler::stop_all() {
    std::scoped_lock lock(mutex_);
    subscriptions_.clear();
}

std::size_t system_data_scheduler::active_count() const noexcept {
    std::scoped_lock lock(mutex_);
    return subscriptions_.size();
}

} /// namespace scheduler
