#pragma once

#include <string>

struct ScanResult {
	bool detected = false;
	std::string ruleName;
	std::string detectorName;
};

class IDetector {
public:
	virtual ~IDetector() = default;
	virtual ScanResult scan(const std::string& filePath) = 0;
};