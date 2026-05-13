#pragma once
#include <string>
#include <memory>
#include <spdlog/logger.h>

class Logger {
public:
	static void init();
	static void shutdown();

	static void info(std::string s);
	static void warn(std::string s);
	static void error(std::string s);

private:
	static std::shared_ptr<spdlog::logger> s_logger;
};


