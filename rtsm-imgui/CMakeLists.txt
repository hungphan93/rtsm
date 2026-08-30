# Fetch GLFW first so ImGui can link to it
include(FetchContent)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    URL https://github.com/glfw/glfw/archive/refs/tags/3.4.zip
)
FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

FetchContent_Declare(
    imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.90.4.zip
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR} 
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

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
