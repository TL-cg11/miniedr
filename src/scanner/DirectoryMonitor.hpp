#pragma once

#include "core/EventBus.hpp"
#include <string>

class DirectoryMonitor {
public:
	DirectoryMonitor() = default;
	~DirectoryMonitor() = default;

	// 감시 시작
	bool start(const std::wstring& watchPath, EventBus& bus);
private:
};