#include <Windows.h>
#include "core/Logger.hpp"
#include "core/Event.hpp"
#include "core/EventBus.hpp"
#include "storage/Database.hpp"
#include "detection/HashDetector.hpp"
#include "detection/YaraDetector.hpp"

// TODO: Phase 3에서 실제 이벤트 소스로 교체
Event makeDemoEvent();

int main() {
	SetConsoleOutputCP(CP_UTF8);		// 콘솔 UTF-8로 인코딩

	Logger::init();

	{
		Database db("events.db");

		EventBus bus;

		bus.subscribe(EventType::FileCreated, [](const Event& e) {
			Logger::logEvent(e);
			});
		bus.subscribe(EventType::FileCreated, [&db](const Event& e) {
			db.insertEvent(e);
			});

		bus.publish(makeDemoEvent());
	}

	{
		YaraDetector yaraDet;
		Logger::info("YaraDetector created");
		if (yaraDet.loadRule(R"(C:\Dev\miniedr\rules\test.yar)")) {
			Logger::info("YARA rules loaded successfully");
		}
		else {
			Logger::error("YARA rules load failed");
		}

		ScanResult yr = yaraDet.scan("C:/EDR-Test/samples/eicar.com.txt");
		Logger::info("Yara scan detected: " + std::to_string(yr.detected) +
			", rule: " + yr.ruleName);
	}

	Logger::shutdown();

	HashDetector det;
	if (det.loadBlacklist("C:/EDR-Test/hashes.txt")) {
		ScanResult r = det.scan("C:/EDR-Test/samples/sha256Test.txt");
		Logger::info("Scan detected: " + std::to_string(r.detected) + ", rule: " + r.ruleName);
	}

	system("pause");
	return 0;
}

Event makeDemoEvent() {
	Event e;
	e.timestamp = std::chrono::system_clock::now();
	e.type = EventType::FileCreated;
	e.source = "DemoSource";
	e.severity = Severity::Warning;
	e.message = "Demo Event";
	e.file_path = "C:\\test\\malware.exe";

	return e;
}