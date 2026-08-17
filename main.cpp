/// MIT License

#include <csignal>
#include <chrono>
#include <thread>
#include <iostream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <functional>
#include <unistd.h> // Bắt buộc cho write() và _exit()

import adapter;
import system_data_scheduler;
import usecase;
import presenter;

/// Xử lý tín hiệu tắt an toàn khẩn cấp (SIGINT từ Ctrl+C hoặc SIGTERM)
extern "C" void signal_handler(int /*signum*/)
{
	// Sử dụng lệnh system call `write` để đảm bảo an toàn tuyệt đối (async-signal-safe).
	// KHÔNG dùng std::cout ở đây vì rất dễ gây deadlock nếu luồng chính đang giữ khóa print_mtx_.
	const char reset_str[] = "\033[2J\033[1;1H\033[?25h\n";
	[[maybe_unused]] auto res = write(STDOUT_FILENO, reset_str, sizeof(reset_str) - 1);
	
	// Bắt buộc ngắt chương trình ngay lập tức (tránh bị treo/hang bởi các thread ngầm của NVML/Scheduler)
	_exit(0);
}

// ========================================================================
// LỚP TERMINAL VIEW
// ========================================================================
class TerminalView {
public:
	TerminalView() {
		std::cout << "\033[?25l"; // Ẩn con trỏ chuột
	}

	~TerminalView() {
		std::cout << "\033[2J\033[1;1H"; // Xóa màn hình khi thoát
		std::cout << "\033[?25h"; // Khôi phục con trỏ chuột
		std::cout << std::flush;
	}

	void render(std::shared_ptr<const presenter::cpu_view_model> cpu,
	            std::shared_ptr<const presenter::memory_view_model> ram,
	            std::shared_ptr<const presenter::gpu_view_model> gpu,
	            std::shared_ptr<const presenter::disk_view_model> disk,
	            std::shared_ptr<const presenter::net_view_model> net)
	{
		std::lock_guard<std::mutex> lock(print_mtx_);
		
		// \033[H  : Đưa con trỏ lên góc trái trên cùng
		// \033[2J : Xóa toàn bộ màn hình hiển thị
		// \033[3J : Xóa sạch luôn cả bộ đệm cuộn (scrollback buffer)
		std::cout << "\033[H\033[2J\033[3J";

		std::cout << "========================================\n";
		std::cout << "        RTSM - TERMINAL MONITOR         \n";
		std::cout << "========================================\n\n";

		if (cpu) {
			std::cout << "[CPU]   Usage: " << std::left << std::setw(15) << cpu->usage_percent 
			          << "Temp: " << cpu->temperature_c << "\n";
		}
		if (ram) {
			std::cout << "[RAM]   Used : " << std::left << std::setw(15) << ram->vram_used 
			          << "Total: " << ram->vram_total << "\n";
		}
		if (gpu) {
			std::cout << "[GPU]   Usage: " << std::left << std::setw(15) << gpu->usage_percent 
			          << "Temp: " << gpu->temperature_c << "\n";
		}
		if (disk) {
			std::cout << "[DISK]  Read : " << std::left << std::setw(15) << disk->read_speed 
			          << "Write: " << disk->write_speed << "\n";
		}
		if (net) {
			std::cout << "[NET]   Down : " << std::left << std::setw(15) << net->rx_speed 
			          << "Up   : " << net->tx_speed << "\n";
		}

		std::cout << std::flush;
	}

private:
	std::mutex print_mtx_;
};

// ========================================================================
// HÀM MAIN
// ========================================================================
int main(int argc, char *argv[])
{
	using namespace std::chrono_literals;

	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);

	// 1. Khởi tạo Core Logic (Lưu ý: Bỏ qua Backend Qt)
	adapter::linux2::system_info_reader_linux reader;
	auto monitor_presenter = std::make_shared<presenter::system_monitor_presenter>();
	usecase::system_monitor_interactor interactor(reader, *monitor_presenter);

	// 2. Khởi tạo Terminal UI
	auto view = std::make_shared<TerminalView>();

	// 3. Khởi chạy bộ lập lịch (Scheduler) lấy dữ liệu định kỳ
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

	// 4. Vòng lặp chính chờ đợi và Cập nhật UI (quét mỗi 500ms)
	while (true) {
		view->render(
			monitor_presenter->cpu_vm(),
			monitor_presenter->memory_vm(),
			monitor_presenter->gpu_vm(),
			monitor_presenter->disk_vm(),
			monitor_presenter->net_vm()
		);
		std::this_thread::sleep_for(500ms);
	}

	return 0;
}