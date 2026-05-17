/// MIT
module;

#include <memory>

export module util:io;

import std;

export namespace util::io
{

namespace fs = std::filesystem;

std::string read_line(const fs::path &path)
{
	std::ifstream file(path);
	std::string line;
	std::getline(file, line);

	return line;
};

std::string exec_cmd(const char *cmd)
{
	std::array<char, 128> buffer;
	std::string result;
	using pipe_close_t = int (*)(FILE *);
	std::unique_ptr<FILE, pipe_close_t> pipe(popen(cmd, "r"),
						 static_cast<pipe_close_t>(pclose));

	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}

	return result;
}

} /// namespace util::io
