cmake_minimum_required(VERSION 4.3.0)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")

set(CMAKE_CXX_MODULE_STD ON)

project(rtsm VERSION 1.00 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


find_package(Qt6 REQUIRED COMPONENTS Quick)
qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(X11 REQUIRED)
endif()

set(QT_FORCE_CMP0156_TO_VALUE NEW)

qt_add_executable(apprtsm main.cpp)

#target_compile_features(apprtsm PRIVATE cxx_std_23)


set_target_properties(apprtsm PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

target_sources(apprtsm PRIVATE
    ### --- Code Files (.cpp) ---
    ui/qt/system_monitor_view_qt.cpp
    scheduler/system_data_scheduler.cpp

    ### --- Header Files (.hpp) ---
    entity/cpu.hpp
    entity/disk.hpp
    entity/gpu.hpp
    entity/memory.hpp
    entity/net.hpp
    use_case/ports/system_info_reader.hpp
    ui/qt/system_monitor_view_qt.hpp
    presenter/system_monitor_presenter.hpp
    platform/window_sticky.hpp
    use_case/ports/system_monitor_input_boundary.hpp
    use_case/ports/system_monitor_output_boundary.hpp
    use_case/system_monitor_interactor.hpp
    scheduler/system_data_scheduler.hpp
    presenter/system_monitor_view_model.hpp
)

### 3.2. OS-specific source files (Prevents cross-compilation linking errors)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_sources(apprtsm PRIVATE
        adapter/linux/system_info_reader_linux.cpp
        adapter/linux/system_info_reader_linux.hpp
        adapter/linux/detail/system_info_reader_linux_detail.hpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_sources(apprtsm PRIVATE
        adapter/window/system_info_reader_window.cpp
        adapter/window/system_info_reader_window.hpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin") ### macOS
    target_sources(apprtsm PRIVATE
        adapter/mac/system_info_reader_mac.cpp
        adapter/mac/system_info_reader_mac.hpp
    )
endif()

qt_add_qml_module(apprtsm
    URI rtsm
    VERSION 1.0
    QML_FILES ui/qt/qml/Main.qml
)

target_include_directories(apprtsm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(apprtsm PRIVATE Qt6::Quick)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_include_directories(apprtsm PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(apprtsm PRIVATE ${X11_LIBRARIES})
endif()

include(GNUInstallDirs)
install(TARGETS apprtsm
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
