#ifndef PRESENTER_SYSTEM_MONITOR_HPP
#define PRESENTER_SYSTEM_MONITOR_HPP
#include "use_case/system_info_reader.hpp"

namespace presenter {

class system_monitor {

public:
    explicit system_monitor(const usecase::system_info_reader& reader) noexcept;

    entity::cpu cpu() const;
    entity::memory memory() const;
    entity::gpu gpu() const;
    entity::disk disk() const;
    entity::net net() const;

private:
    const usecase::system_info_reader& reader_;
};

} // namespace presenter

#endif // PRESENTER_SYSTEM_MONITOR_HPP
