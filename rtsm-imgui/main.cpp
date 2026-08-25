/// MIT License
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <filesystem>
#include <unistd.h>
#include <linux/limits.h>
#include <string>
#include <iostream>
#include <csignal>
#include <memory>
#include <fcntl.h>
#include <sys/file.h>
#include <stdio.h>
#include <cstring>

#include <GLFW/glfw3.h>
#include "platform/window_sticky.hpp"

import adapter;
import system_data_scheduler;
import usecase;
import presenter;

static bool g_running = true;

static void signal_handler(int signum)
{
    const char* sig_name = "UNKNOWN";
    if (signum == SIGINT) sig_name = "SIGINT";
    else if (signum == SIGTERM) sig_name = "SIGTERM";
    else if (signum == SIGTSTP) sig_name = "SIGTSTP (Ctrl+Z)";
    else if (signum == SIGQUIT) sig_name = "SIGQUIT";

    std::cout << "\nReceived " << sig_name << ". Quitting gracefully...\n";
    g_running = false;
}

static void glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ": " << description << "\n";
}

std::string get_font_path() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    std::string exe_dir = ".";
    if (count != -1) {
        std::string path(result, count);
        exe_dir = path.substr(0, path.find_last_of('/'));
    }

    std::string bundled_path = exe_dir + "/../share/rtsm-imgui/fonts/Roboto-Medium.ttf";
    std::string dev_path = exe_dir + "/../../../rtsm-imgui/imgui/misc/fonts/Roboto-Medium.ttf";
    
    if (std::filesystem::exists(bundled_path)) return bundled_path;
    if (std::filesystem::exists(dev_path)) return dev_path;
    return "rtsm-imgui/imgui/misc/fonts/Roboto-Medium.ttf"; // Fallback
}

int main()
{
    using namespace std::chrono_literals;

    /// Ensure only one instance of the application is running at a time
    int lock_file = open("/tmp/rtsm_imgui.lock", O_CREAT | O_RDWR, 0666);
    if (lock_file == -1 || flock(lock_file, LOCK_EX | LOCK_NB) == -1) {
        std::cerr << "Another instance is already running.\n";
        return 0;
    }

    /// Handle IDE and terminal Stop commands gracefully
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGTSTP, signal_handler); /// Handle Ctrl+Z
    std::signal(SIGQUIT, signal_handler); /// Handle Ctrl+

    /// Force X11 so we can use X11 sticky window atoms (Wayland LayerShell unsupported)
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

#if defined(__APPLE__)
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE); /// Let clicks pass through to desktop below

    /// Get primary monitor resolution to span the entire screen width
    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
    int screen_w = mode ? mode->width : 1920;
    
    /// Create window with graphics context (span full screen width, slightly taller for bigger font)
    GLFWwindow* window = glfwCreateWindow(screen_w, 400, "Real-time system monitoring", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    
    glfwSetWindowPos(window, 0, 40);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); /// Enable vsync

    /// Apply sticky window logic for Linux/X11
    platform::make_window_sticky(window);

    /// Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    (void)io;
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImFont* custom_font = io.Fonts->AddFontFromFileTTF(get_font_path().c_str(), 45.0f);
    if (custom_font == nullptr) {
        std::cerr << "Failed to load custom bold font!\n";
    }

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    adapter::linux2::system_info_reader_linux reader;
    auto presenter = std::make_shared<presenter::system_monitor_presenter>();
    usecase::system_monitor_interactor interactor(reader, *presenter);

    scheduler::system_data_scheduler data_scheduler;

    /// Register periodic data sampling tasks
    [[maybe_unused]] auto t1_ = data_scheduler.subscribe(300ms, [&interactor]() {
        interactor.fetch_cpu();
    });
    [[maybe_unused]] auto t2_ = data_scheduler.subscribe(500ms, [&interactor]() {
        interactor.fetch_memory();
    });
    [[maybe_unused]] auto t3_ = data_scheduler.subscribe(500ms, [&interactor]() {
        interactor.fetch_gpu();
    });
    [[maybe_unused]] auto t4_ = data_scheduler.subscribe(1000ms, [&interactor]() {
        interactor.fetch_disk();
    });
    [[maybe_unused]] auto t5_ = data_scheduler.subscribe(1000ms, [&interactor]() {
        interactor.fetch_net();
    });

    /// Main loop
    while (!glfwWindowShouldClose(window) && g_running)
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(screen_w, 400), ImGuiCond_Always);
        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | 
                                        ImGuiWindowFlags_NoBackground | 
                                        ImGuiWindowFlags_NoSavedSettings | 
                                        ImGuiWindowFlags_NoFocusOnAppearing | 
                                        ImGuiWindowFlags_NoNav |
                                        ImGuiWindowFlags_NoInputs |
                                        ImGuiWindowFlags_NoMove;
        
        ImGui::Begin("RTSM Dashboard Overlay", nullptr, window_flags);

        ImVec4 color_device_name(0.0f, 0.631f, 0.027f, 1.0f);
        ImVec4 color_device_detail(0.902f, 0.0f, 0.843f, 1.0f);

        float row_height = 400.0f / 5.0f;
        float text_y_offset = (row_height - ImGui::GetTextLineHeight()) * 0.5f;

        /// Base positions for the first 3 detail columns
        float col_x[] = { screen_w * 0.28f, screen_w * 0.46f, screen_w * 0.64f }; 

        auto draw_row = [&](int row_index, const std::string& title, const std::vector<std::string>& details) {
            float base_y = row_index * row_height + text_y_offset;
            ImGui::SetCursorPos(ImVec2(50.0f, base_y)); // 50px left margin
            ImGui::TextColored(color_device_name, "%s", title.c_str());
            
            for (size_t i = 0; i < details.size(); ++i) {
                if (i < 3) {
                    ImGui::SetCursorPos(ImVec2(col_x[i], base_y));
                    ImGui::TextColored(color_device_detail, "%s", details[i].c_str());
                } else if (i == 3) {
                    /// Right align the 4th column exactly 100px from the right edge
                    ImVec2 text_size = ImGui::CalcTextSize(details[i].c_str());
                    ImGui::SetCursorPos(ImVec2(screen_w - 100.0f - text_size.x, base_y));
                    ImGui::TextColored(color_device_detail, "%s", details[i].c_str());
                }
            }
        };

        auto cpu_vm = presenter->cpu_vm();
        draw_row(0, cpu_vm->model_name, {cpu_vm->usage_percent, cpu_vm->temperature_c, cpu_vm->power, cpu_vm->frequency_mhz});

        auto mem_vm = presenter->memory_vm();
        draw_row(1, mem_vm->name.empty() ? "Ram" : mem_vm->name, {mem_vm->usage_percent, mem_vm->voltage, mem_vm->vram_used + mem_vm->vram_total, mem_vm->frequency_mhz});

        auto gpu_vm = presenter->gpu_vm();
        draw_row(2, gpu_vm->name, {gpu_vm->usage_percent, gpu_vm->temperature_c, gpu_vm->vram_used + gpu_vm->vram_total, gpu_vm->frequency_mhz});

        auto disk_vm = presenter->disk_vm();
        draw_row(3, disk_vm->model, {disk_vm->usage_percent, "Read: " + disk_vm->read_speed + " | Write: " + disk_vm->write_speed});
        
        auto net_vm = presenter->net_vm();
        draw_row(4, "Network", {"Down: " + net_vm->rx_speed + " | Up: " + net_vm->tx_speed});

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    
    if (lock_file != -1) {
        close(lock_file);
        remove("/tmp/rtsm_imgui.lock");
    }

    return 0;
}
