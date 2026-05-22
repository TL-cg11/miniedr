#pragma once

#include "detection/IDetector.hpp"
#include <string> // std::string
#include <chrono> // std::chrono::system_clock
#include <optional> // std::optional

using TimePoint = std::chrono::system_clock::time_point; // typedef

enum class EventType {
	Unknown,
	FileCreated,
	FileModified,
	ScanRequested,
	ThreatDetected,
	FileQuarantined,
	AgentStarted,
	AgentStopped,
	ErrorOccurred,
};

enum class Severity {
	Info,
	Warning,
	Error,
};

struct Event {
	TimePoint timestamp;							// 언제
	EventType type = EventType::Unknown;				// 무엇이
	std::string source;								// 어디서
	Severity severity = Severity::Info;			// 얼마나 심각한지
	std::string message;							// 세부적인 설명

	std::optional<std::string> file_path;			// 관련 파일 경로
	std::optional<std::string> rule_name;			// 룰 매칭
	std::optional<std::string> quarantine_path;		// 격리된 위치
};

std::string eventTypeToString(EventType type);
std::string severityToString(Severity severity);
std::string timePointToString(TimePoint timestamp);

Event makeDetectionEvent(const ScanResult& result, 
	const std::string& filePath);

Event makeFileSystemEvent(EventType type, const std::string& filePath);

std::string timePointToIso8601(TimePoint timestamp);
std::string eventToJson(const Event& e);