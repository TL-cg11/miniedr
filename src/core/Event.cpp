#include "Event.hpp"
#include <ctime>		// time_t, tm, localtime_s, strftime
#include <sstream>		// std::ostringstream
#include <iomanip>		// std::put_time

std::string eventTypeToString(EventType type) {
	switch (type) {
	case EventType::Unknown: return "Unknown";
	case EventType::FileCreated: return "FileCreated";
	case EventType::FileModified: return "FileModified";
	case EventType::ScanRequested: return "ScanRequested";
	case EventType::ThreatDetected: return "ThreatDetected";
	case EventType::FileQuarantined: return "FileQuarantined";
	case EventType::AgentStarted: return "AgentStarted";
	case EventType::AgentStopped: return "AgentStopped";
	case EventType::ErrorOccurred: return "ErrorOccurred";
	default: return "Unknown";
	}
}

std::string severityToString(Severity severity) {
	switch (severity) {
	case Severity::Info: return "Info";
	case Severity::Warning: return "Warning";
	case Severity::Error: return "Error";
	default: return "Info";
	}
}

std::string timePointToString(TimePoint timestamp) {

	time_t tt = std::chrono::system_clock::to_time_t(timestamp);

	std::tm t;
	localtime_s(&t, &tt);

	std::ostringstream oss;
	oss << std::put_time(&t, "%Y-%m-%d %H:%M:%S");

	return oss.str();
}