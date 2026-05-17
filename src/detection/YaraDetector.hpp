#pragma once

#include "IDetector.hpp"
#include <string>
#include <yara.h>

class YaraDetector : public IDetector {
public:
	YaraDetector();
	~YaraDetector();

	bool loadRule(const std::string& rulePath);

	ScanResult scan(const std::string& filePath) override;
private:
	YR_RULES* m_rules = nullptr;

	static int yaraCallback(YR_SCAN_CONTEXT* context,
		int message, void* message_data, void* user_data);
};