#include "core/Logger.hpp"
#include <spdlog/spdlog.h>

int main() {
	Logger::init();

	Logger::info("Hello, MiniEDR!");
	Logger::warn("This is a warning log.");
	Logger::error("This is an error log.");

	Logger::shutdown();

	system("pause"); // 디버깅
	return 0;
}