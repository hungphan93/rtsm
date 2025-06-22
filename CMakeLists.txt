cmake_minimum_required(VERSION 3.16)

cmake_policy(SET CMP0091 NEW)
project(rtsm VERSION 0.1 LANGUAGES CXX)

#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#google test library
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.5)

qt_policy(SET QTP0004 NEW)

qt_add_executable(apprtsm
    main.cpp
)

qt_add_qml_module(apprtsm
    URI rtsm
    VERSION 1.0
    SOURCES
        entity/cpu.hpp
        entities/ecpuinfo.h
        entities/ememoryinfo.h
        entities/egpuinfo.h
        entities/esysteminfo.h
        entities/ediskinfo.h
        entities/enetworkinfo.h
        interfaces/isysteminforeader.h
        usecases/ucsysteminforeader.h
        usecases/ucsysteminforeader.cc
        adapters/adlinuxsysteminforeader.h
        adapters/adlinuxsysteminforeader.cpp
        adapters/adapterfetchmanager.h
        adapters/adapterfetchmanager.cpp
        presentation/psystemmonitor.h
        presentation/psystemmonitor.cpp
        interfaces/isysteminfologger.h
    QML_FILES
        ui/Main.qml
        SOURCES entity/disk.hpp
        use_case/system_info_reader.hpp
        SOURCES entity/gpu.hpp
        SOURCES entity/memory.hpp
        SOURCES entity/net.hpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(apprtsm PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.apprtsm
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(apprtsm
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS apprtsm
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

enable_testing()
# Google Test + unit tests
add_subdirectory(tests)
