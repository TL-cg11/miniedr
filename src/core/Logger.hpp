#pragma once
#include <string>
#include <memory>
#include <spdlog/logger.h>
#include "Event.hpp"

class Logger {
public:
	static void init();
	static void shutdown();

	static void logEvent(const Event& e);
	static void info(const std::string& s);
	static void warn(const std::string& s);
	static void error(const std::string& s);

private:
	static std::shared_ptr<spdlog::logger> s_logger;
};


