# Fetch GLFW first so ImGui can link to it
include(FetchContent)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

# Build ImGui as a standalone static library module
add_subdirectory(imgui)

# Build the main executable
add_executable(rtsm-imgui main.cpp)

target_sources(rtsm-imgui
    PRIVATE FILE_SET CXX_MODULES FILES
    system_monitor_view_imgui.ixx
)

# Link ImGui and RTSM::Core (just like Qt!)
target_link_libraries(rtsm-imgui PRIVATE
    imgui
    RTSM::Core
)

install(TARGETS rtsm-imgui DESTINATION bin COMPONENT rtsm-imgui)
install(DIRECTORY fonts/ DESTINATION share/rtsm-imgui/fonts COMPONENT rtsm-imgui)
