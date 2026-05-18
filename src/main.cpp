#include <Windows.h>
#include "core/Logger.hpp"
#include "core/Event.hpp"
#include "core/EventBus.hpp"
#include "storage/Database.hpp"
#include "detection/HashDetector.hpp"
#include "detection/YaraDetector.hpp"

int main() {
	SetConsoleOutputCP(CP_UTF8);		// 콘솔 UTF-8로 인코딩

	Logger::init();

	{
		Database db("events.db");

		EventBus bus;

		bus.subscribe(EventType::ThreatDetected, [](const Event& e) {
			Logger::logEvent(e);
			});
		bus.subscribe(EventType::ThreatDetected, [&db](const Event& e) {
			db.insertEvent(e);
			});

		std::string filePath = "C:/EDR-Test/samples/eicar.com.txt";

		HashDetector hashDet;
		if (hashDet.loadBlacklist("C:/EDR-Test/hashes.txt")) {
			Logger::info("Hash Blacklist loaded successfully");
		}
		else {
			Logger::error("Hash Blacklist load failed");
		}
		ScanResult hr = hashDet.scan(filePath);
		if (hr.detected) {
			bus.publish(makeDetectionEvent(hr, filePath));
		}

		YaraDetector yaraDet;
		Logger::info("YaraDetector created");
		if (yaraDet.loadRule(R"(C:\Dev\miniedr\rules\test.yar)")) {
			Logger::info("YARA rules loaded successfully");
		}
		else {
			Logger::error("YARA rules load failed");
		}
		ScanResult yr = yaraDet.scan(filePath);
		if (yr.detected) {
			bus.publish(makeDetectionEvent(yr, filePath));
		}
	}

	Logger::shutdown();

	system("pause");
	return 0;
}