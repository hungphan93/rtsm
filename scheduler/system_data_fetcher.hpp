/// MIT License
#ifndef SYSTEM_DATA_FETCHER_HPP
#define SYSTEM_DATA_FETCHER_HPP
#include "use_case/ports/system_info_reader.hpp"

namespace scheduler {

class system_data_fetcher {

public:
    explicit system_data_fetcher(const usecase::system_info_reader& reader) noexcept;
    ~system_data_fetcher() noexcept = default;

    system_data_fetcher(const system_data_fetcher&) = delete;
    system_data_fetcher(system_data_fetcher&&) = delete;
    system_data_fetcher& operator=(const system_data_fetcher&) = delete;
    system_data_fetcher& operator=(system_data_fetcher&&) = delete;

    [[nodiscard]] entity::cpu fetch_cpu() const;
    [[nodiscard]] entity::memory fetch_memory() const;
    [[nodiscard]] entity::gpu fetch_gpu() const;
    [[nodiscard]] entity::disk fetch_disk() const;
    [[nodiscard]] entity::net fetch_net() const;

private:
    const usecase::system_info_reader& reader_;
};

} /// namespace scheduler

#endif /// SYSTEM_DATA_FETCHER_HPP
