/// MIT License
#include <csignal>
#include <memory>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include <iostream>

import std;
import adapter;
import system_data_scheduler;
import usecase;
import presenter;

import system_monitor_view_imgui;

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

    /// Initialize Core Logic
    adapter::linux2::system_info_reader_linux reader;
    auto presenter = std::make_shared<presenter::system_monitor_presenter>();
    usecase::system_monitor_interactor interactor(reader, *presenter);

    /// Initialize ImGui UI
    system_monitor_view_imgui view;

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
    while (!view.should_close() && g_running)
    {
        view.render(presenter->cpu_vm(),
                    presenter->memory_vm(),
                    presenter->gpu_vm(),
                    presenter->disk_vm(),
                    presenter->net_vm());
    }

    if (lock_file != -1) {
        close(lock_file);
        remove("/tmp/rtsm_imgui.lock");
    }

    return 0;
}
