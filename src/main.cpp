#include <spdlog/spdlog.h>

int main() {
	spdlog::info("Hello, MiniEDR!");
	spdlog::warn("This is a warning log.");
	spdlog::error("This is an error log.");
	return 0;
}