cmake_minimum_required(VERSION 4.3.0)

project(rtsm VERSION 1.00 LANGUAGES CXX)

### ==============================================================================
### 1. FIND DEPENDENCIES
### ==============================================================================
find_package(Qt6 REQUIRED COMPONENTS Quick)
qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(X11 REQUIRED)
endif()

### ==============================================================================
### 2. TARGET INITIALIZATION & C++ STANDARD
### ==============================================================================
### Ensure CMP0156 compatibility with modern CMake versions (4.3.0)
set(QT_FORCE_CMP0156_TO_VALUE NEW)

### Initialize executable with the main entry file
qt_add_executable(apprtsm main.cpp)

### Enforce cxx_std_26 standard requirement (Compiler/Environment details handled via CMakePresets.json)
target_compile_features(apprtsm PRIVATE cxx_std_26)


set_target_properties(apprtsm PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

### ==============================================================================
### 3. SOURCE MANAGEMENT
### ==============================================================================

### 3.1. Common source files (Cross-platform)
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

### ==============================================================================
### 4. QML MODULE MANAGEMENT
### ==============================================================================
qt_add_qml_module(apprtsm
    URI rtsm
    VERSION 1.0
    QML_FILES ui/qt/qml/Main.qml
)

### ==============================================================================
### 5. INCLUDE AND LINKING CONFIGURATION
### ==============================================================================
### Add project root as include directory
target_include_directories(apprtsm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

### Link Qt6 libraries
target_link_libraries(apprtsm PRIVATE Qt6::Quick)

### Link X11 libraries on Linux
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_include_directories(apprtsm PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(apprtsm PRIVATE ${X11_LIBRARIES})
endif()

### ==============================================================================
### 6. INSTALLATION CONFIGURATION
### ==============================================================================
include(GNUInstallDirs)
install(TARGETS apprtsm
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
