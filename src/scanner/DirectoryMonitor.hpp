#pragma once

#include "core/EventBus.hpp"
#include <string>
#include <atomic>
#include <Windows.h>

class DirectoryMonitor {
public:
	DirectoryMonitor() = default;
	~DirectoryMonitor();

	// 복사 및 이동 금지
	DirectoryMonitor(const DirectoryMonitor&) = delete;
	DirectoryMonitor& operator = (const DirectoryMonitor&) = delete;

	// 감시 시작
	bool start(const std::wstring& watchPath, EventBus& bus);
	void stop();
private:
	std::atomic<bool> m_running{ true };
	HANDLE m_hDir = INVALID_HANDLE_VALUE;
	HANDLE m_hChangeEvent = NULL;
	HANDLE m_hStopEvent = NULL;
};