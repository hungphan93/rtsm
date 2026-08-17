add_library(rtsm-core STATIC)
add_library(RTSM::Core ALIAS rtsm-core)

target_sources(rtsm-core
    PUBLIC
       FILE_SET CXX_MODULES
            FILES
                util/util.ixx
                util/parse.ixx
                util/convert.ixx
                util/io.ixx

                entity/entity.ixx
                entity/cpu.ixx
                entity/disk.ixx
                entity/gpu.ixx
                entity/memory.ixx
                entity/net.ixx

                use_case/usecase.ixx
                use_case/ports/system_info_reader.ixx
                use_case/ports/system_monitor_input_boundary.ixx
                use_case/ports/system_monitor_output_boundary.ixx
                use_case/system_monitor_interactor.ixx

                presenter/presenter.ixx
                presenter/system_monitor_presenter.ixx
                presenter/system_monitor_view_model.ixx

                scheduler/system_data_scheduler.ixx
    PRIVATE
                scheduler/system_data_scheduler.cpp
                platform/window_sticky.hpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(X11 REQUIRED)
    target_sources(rtsm-core 
        PUBLIC
            FILE_SET CXX_MODULES
                FILES
                    adapter/linux/system_info_reader_linux.ixx
                    adapter/linux/adapter.ixx
                    adapter/linux/detail/system_info_reader_linux_detail.ixx
                    adapter/linux/detail/nvml.ixx

        PRIVATE
            adapter/linux/system_info_reader_linux.cpp
    )
    target_include_directories(rtsm-core PUBLIC ${X11_INCLUDE_DIR})
    target_link_libraries(rtsm-core PUBLIC ${X11_LIBRARIES})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_sources(rtsm-core 
        PRIVATE
            adapter/window/system_info_reader_window.cpp
            adapter/window/system_info_reader_window.hpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_sources(rtsm-core
        PRIVATE
            adapter/mac/system_info_reader_mac.cpp
            adapter/mac/system_info_reader_mac.hpp
    )
endif()

# Export include dir so qt/cli can find platform/window_sticky.hpp etc.
target_include_directories(rtsm-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
