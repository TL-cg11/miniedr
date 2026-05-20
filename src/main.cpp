#include <Windows.h>
#include "core/Logger.hpp"
#include "core/Event.hpp"
#include "core/EventBus.hpp"
#include "storage/Database.hpp"
#include "detection/HashDetector.hpp"
#include "detection/YaraDetector.hpp"
#include "scanner/FileScanner.hpp"
#include "core/StringUtil.hpp"
#include "scanner/DirectoryMonitor.hpp"


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

		HashDetector hashDet;
		if (hashDet.loadBlacklist("C:/EDR-Test/hashes.txt")) {
			Logger::info("Hash Blacklist loaded successfully");
		}
		else {
			Logger::error("Hash Blacklist load failed");
		}

		YaraDetector yaraDet;
		Logger::info("YaraDetector created");
		if (yaraDet.loadRule(R"(C:\Dev\miniedr\rules\test.yar)")) {
			Logger::info("YARA rules loaded successfully");
		}
		else {
			Logger::error("YARA rules load failed");
		}

		FileScanner scanner;
		auto files = scanner.scan("C:/EDR-Test/watch");

		Logger::info("FileScanner found " + std::to_string(files.size()) + " files");
		for (const auto& f : files) {
			Logger::info("  " + f);
			ScanResult hr = hashDet.scan(f);
			if (hr.detected) {
				bus.publish(makeDetectionEvent(hr, f));
			}

			ScanResult yr = yaraDet.scan(f);
			if (yr.detected) {
				bus.publish(makeDetectionEvent(yr, f));
			}
		}

		auto onFileChange = [&](const Event& e) {
			if (!e.file_path) {
				Logger::warn("File event without path");
				return;
			}
			const std::string& path = *e.file_path;

			Logger::info("Auto-scanning: " + path);

			ScanResult hr = hashDet.scan(path);
			if (hr.detected) {
				bus.publish(makeDetectionEvent(hr, path));
			}

			ScanResult yr = yaraDet.scan(path);
			if (yr.detected) {
				bus.publish(makeDetectionEvent(yr, path));
			}
		};

		bus.subscribe(EventType::FileCreated, onFileChange);
		bus.subscribe(EventType::FileModified, onFileChange);

		Logger::info("=== Starting real-time directory monitoring ===");
		DirectoryMonitor monitor;
		monitor.start(L"C:/EDR-Test/watch", bus);
	}

	Logger::shutdown();

	system("pause");
	return 0;
}