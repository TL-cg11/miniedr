#include <Windows.h>
#include "core/Logger.hpp"
#include <spdlog/spdlog.h>
#include "core/Event.hpp"
#include "core/EventBus.hpp"

Event EventTest();

int main() {
	SetConsoleOutputCP(CP_UTF8);		// 콘솔 UTF-8로 인코딩

	Logger::init();

	Logger::info("Hello, MiniEDR!");
	Logger::warn("This is a warning log.");
	Logger::error("This is an error log.");

	//EventTest();
	EventBus bus;

	bus.subscribe(EventType::FileCreated, [](const Event& e) {
		Logger::logEvent(e);
		});
	bus.subscribe(EventType::FileCreated, [](const Event& e) {
		Logger::logEvent(e);
		});

	bus.publish(EventTest());

	Logger::shutdown();

	system("pause"); // 디버깅
	return 0;
}

Event EventTest() {
	Event e;
	e.timestamp = std::chrono::system_clock::now();
	e.type = EventType::FileCreated;
	e.source = "DirectoryMonitor";
	e.severity = Severity::Warning;
	e.message = "New File";
	e.file_path = "C:\\test\\malware.exe";

	return e;

	//Logger::warn("Event: " + e.source + " - " + e.message);
}