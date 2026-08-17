/// MIT License

#include <csignal>
#include <chrono>
#include <thread>
#include <iostream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <functional>
#include <unistd.h> // Required for write() and _exit()

import adapter;
import system_data_scheduler;
import usecase;
import presenter;

/// Handle emergency safe shutdown signals (SIGINT from Ctrl+C, SIGTERM, SIGTSTP)
extern "C" void signal_handler(int /*signum*/)
{
	/// Use the write system call to guarantee absolute async-signal safety.
	/// DO NOT use std::cout here, as it can cause deadlocks if the main thread holds print_mtx_.
	const char reset_str[] = "\033[2J\033[1;1H\033[?25h\n";
	[[maybe_unused]] auto res = write(STDOUT_FILENO, reset_str, sizeof(reset_str) - 1);

	/// Force immediately terminate the program (avoid hanging by background NVML/Scheduler threads)
	_exit(0);
}

import system_monitor_view_cli;

int main(int argc, char *argv[])
{
	using namespace std::chrono_literals;

	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);
	std::signal(SIGTSTP, signal_handler); // Catch Ctrl+Z signal to exit the program

	/// Initialize Core Logic (Note: Bypassing Qt Backend)
	adapter::linux2::system_info_reader_linux reader;
	auto monitor_presenter = std::make_shared<presenter::system_monitor_presenter>();
	usecase::system_monitor_interactor interactor(reader, *monitor_presenter);

	/// Initialize Terminal UI
	auto view = std::make_shared<system_monitor_view_cli>();

	/// Start the scheduler to fetch data periodically
	scheduler::system_data_scheduler data_scheduler;

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

	/// Fetch data initially (Synchronous) to ensure that during the first render,
	/// all parameters (CPU, RAM, GPU...) appear simultaneously,
	/// preventing "pop-in" effects (where faster loads appear first).
	// interactor.fetch_cpu();
	// interactor.fetch_memory();
	// interactor.fetch_gpu();
	// interactor.fetch_disk();
	// interactor.fetch_net();

	/// Main loop waiting and updating UI (scans every 500ms)
	while (true) {
		view->render(monitor_presenter->cpu_vm(),
			     monitor_presenter->memory_vm(),
			     monitor_presenter->gpu_vm(),
			     monitor_presenter->disk_vm(),
			     monitor_presenter->net_vm());
		std::this_thread::sleep_for(500ms);
	}

	return 0;
}
