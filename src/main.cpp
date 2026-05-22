#include <Windows.h>
#include <thread>
#include <iostream>
#include <string>
#include <fstream>
#include "core/Logger.hpp"
#include "core/Event.hpp"
#include "core/EventBus.hpp"
#include "storage/Database.hpp"
#include "detection/HashDetector.hpp"
#include "detection/YaraDetector.hpp"
#include "scanner/FileScanner.hpp"
#include "core/StringUtil.hpp"
#include "scanner/DirectoryMonitor.hpp"
#include "quarantine/Quarantine.hpp"


int main() {
	SetConsoleOutputCP(CP_UTF8);		// 콘솔 UTF-8로 인코딩

	Logger::init();

	{
		Database db("events.db");

		EventBus bus;

		Quarantine quarantine("quarantine");

		std::ofstream jsonlOut("events.jsonl", std::ios::app);

		bus.subscribe(EventType::ThreatDetected, [](const Event& e) {
			Logger::logEvent(e);
		});
		bus.subscribe(EventType::ThreatDetected, [&quarantine, &db, &jsonlOut](const Event& e) {
			if (!e.file_path) {
				db.insertEvent(e);
				jsonlOut << eventToJson(e) << std::endl;
				return;
			}

			std::string ruleName = e.rule_name.value_or("unknown");
			std::string quarantinePath = quarantine.quarantineFile(*e.file_path, ruleName);

			Event enriched = e;
			if (!quarantinePath.empty()) {
				enriched.quarantine_path = quarantinePath;
			}
			db.insertEvent(enriched);
			jsonlOut << eventToJson(enriched) << std::endl;
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

		DirectoryMonitor monitor;
		std::thread monitorThread(&DirectoryMonitor::start, &monitor, L"C:/EDR-Test/watch", std::ref(bus));

		Logger::info("Monitoring started. Type 'stop' to quit.");

		std::string cmd;
		while (std::cin >> cmd) {
			if (cmd == "stop") {
				break;
			}
			Logger::info("Unknown command: " + cmd);
		}

		Logger::info("Stop requested, shutting down monitor...");
		monitor.stop();
		monitorThread.join();
		Logger::info("Monitor stopped cleanly.");
	}

	Logger::shutdown();

	return 0;
}