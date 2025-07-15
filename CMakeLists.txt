cmake_minimum_required(VERSION 3.24)

project(rtsm VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find X11 package for Linux
if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
endif()

#google test library
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Install your icon
install(FILES icons/app_icon.png DESTINATION share/icons/hicolor/48x48/apps)

# Install your .desktop file
install(FILES rtsm.desktop DESTINATION share/applications)

# Qt Setup
find_package(Qt6 REQUIRED COMPONENTS Quick)
qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

# Source files (CPP only)
set(APP_SOURCES
  adapter/linux/system_info_reader_linux.cpp
  adapter/window/system_info_reader_window.cpp
  adapter/mac/system_info_reader_mac.cpp

  presenter/system_monitor.cpp

  ui/qt/system_monitor_qt.cpp

  scheduler/system_fetch_manager.cpp
)

# Header files (for IDE and includes)
set(APP_HEADERS
  main.cpp

  entity/cpu.hpp
  entity/disk.hpp
  entity/gpu.hpp
  entity/memory.hpp
  entity/net.hpp

  use_case/system_info_reader.hpp

  adapter/linux/system_info_reader_linux.hpp
  adapter/window/system_info_reader_window.hpp
  adapter/mac/system_info_reader_mac.hpp

  presenter/system_monitor.hpp

  ui/qt/system_monitor_qt.hpp

  scheduler/system_fetch_manager.hpp
)

# Add executable target
qt_add_executable(apprtsm
  ${APP_SOURCES}
  ${APP_HEADERS}
)

# Add QML module
qt_add_qml_module(
  apprtsm
  URI rtsm
  VERSION 1.0
  QML_FILES
  ui/qt/qml/Main.qml
  SOURCES platform/window_sticky.hpp
)

# Set bundle metadata (macOS/Windows)
set_target_properties(apprtsm PROPERTIES
  MACOSX_BUNDLE TRUE
  WIN32_EXECUTABLE TRUE
  MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
  MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

# Link Qt libraries
target_link_libraries(apprtsm
  PRIVATE Qt6::Quick
  ${X11_LIBRARIES}
)

# Include directories for X11
if(UNIX AND NOT APPLE)
    target_include_directories(apprtsm PRIVATE
        ${X11_INCLUDE_DIR}
    )
endif()

# Installation (Linux/macOS)
include(GNUInstallDirs)
install(TARGETS apprtsm
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# # Google Test
# enable_testing()
# add_subdirectory(tests)
