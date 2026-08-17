/// MIT License
export module system_monitor_view_cli;

import std;
import presenter;

export class system_monitor_view_cli {
public:
	system_monitor_view_cli()
	{
		std::print("\033[?25l"); /// Hide cursor
		std::print("\033[2J\033[3J\033[H"); /// Clear screen once at startup
	}

	~system_monitor_view_cli()
	{
		std::print("\033[2J\033[1;1H"); /// Clear screen on exit
		std::print("\033[?25h");	/// Restore cursor
		std::fflush(nullptr);
	}

	void render(std::shared_ptr<const presenter::cpu_view_model> cpu,
		    std::shared_ptr<const presenter::memory_view_model> ram,
		    std::shared_ptr<const presenter::gpu_view_model> gpu,
		    std::shared_ptr<const presenter::disk_view_model> disk,
		    std::shared_ptr<const presenter::net_view_model> net)
	{
		std::lock_guard<std::mutex> lock(print_mtx_);

		/// \033[H  : Move cursor to the top-left corner without clearing the screen
		/// This prevents terminal flickering (screen tearing) during fast updates.
		std::print("\033[H\n");

		auto print_row = [](const std::string &name,
				    const std::string &col1 = "",
				    const std::string &col2 = "",
				    const std::string &col3 = "",
				    const std::string &col4 = "") {
			const char *GREEN = "\033[1;32m";
			const char *PINK = "\033[1;35m";
			const char *RESET = "\033[0m";

			auto format_name = [](std::string s) {
				auto pos = s.find_last_not_of(" \n\r\t");
				if (pos != std::string::npos)
					s.erase(pos + 1);
				else
					s.clear();
				if (s.length() > 34)
					s = s.substr(0, 31) + "...";
				return s;
			};

			auto pad_right = [](const std::string &str, int width) {
				int visible_len = 0;
				for (int i = 0; i < str.length(); ++i) {
					if ((str[i] & 0xC0) != 0x80)
						visible_len++;
				}
				return str + std::string(std::max(0, width - visible_len), ' ');
			};

			std::print("{}{}{}", GREEN, pad_right(format_name(name), 35), RESET);
			if (!col1.empty())
				std::print("{}{}{}", PINK, pad_right(col1, 25), RESET);
			if (!col2.empty())
				std::print("{}{}{}", PINK, pad_right(col2, 15), RESET);
			if (!col3.empty())
				std::print("{}{}{}", PINK, pad_right(col3, 25), RESET);
			if (!col4.empty())
				std::print("{}{}{}", PINK, col4, RESET);
			std::print("\n\n");
		};

		if (cpu)
			print_row(cpu->model_name,
				  cpu->usage_percent,
				  cpu->temperature_c,
				  cpu->power,
				  cpu->frequency_mhz);
		if (ram)
			print_row(ram->name,
				  ram->usage_percent,
				  ram->voltage,
				  ram->vram_used + ram->vram_total,
				  ram->frequency_mhz);
		if (gpu)
			print_row(gpu->name,
				  gpu->usage_percent,
				  gpu->temperature_c,
				  gpu->vram_used + gpu->vram_total,
				  gpu->frequency_mhz);
		if (disk)
			print_row(disk->model,
				  disk->usage_percent,
				  "Read: " + disk->read_speed + " | Write: " + disk->write_speed);
		if (net)
			print_row("Network", "Down: " + net->rx_speed + " | Up: " + net->tx_speed);

		/// \033[0J : Clear from cursor to the end of the screen to remove any leftover lines
		std::print("\033[0J");
		std::fflush(nullptr);
	}

private:
	std::mutex print_mtx_;
};
