#include "Logger.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <filesystem>

std::shared_ptr<spdlog::logger> Logger::s_logger;

void Logger::init() {
	std::filesystem::create_directories("logs"); // 로그 폴더 생성

	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
		"logs/miniedr.log",
		true	// 파일 실행마다 초기화 (false : append)
	);

	std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink };
	s_logger = std::make_shared<spdlog::logger>("miniedr", sinks.begin(), sinks.end());


	spdlog::register_logger(s_logger);

	s_logger->set_level(spdlog::level::trace);
	s_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
	s_logger->flush_on(spdlog::level::warn);
}

void Logger::shutdown() {
	if (s_logger) {
		s_logger->flush(); // 버퍼 비우기
	}

	spdlog::drop_all(); // 등록된 logger 해제

	spdlog::shutdown(); // spdlog 자체 종료
}

void Logger::logEvent(const Event& e) {
	switch (e.severity) {
	case Severity::Info:
		Logger::info(e.message);
		break;
	case Severity::Warning:
		Logger::warn(e.message);
		break;
	case Severity::Error:
		Logger::error(e.message);
		break;
	}
}

void Logger::info(const std::string& s) {
	s_logger->info(s);
}
void Logger::warn(const std::string& s) {
	s_logger->warn(s);
}
void Logger::error(const std::string& s) {
	s_logger->error(s);
}