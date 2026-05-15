#pragma once

#include "IDetector.hpp"
#include <string>
#include <unordered_set>

class HashDetector : public IDetector  {
public:
	bool loadBlacklist(const std::string& blacklistPath);

	ScanResult scan(const std::string& filePath) override;

	size_t blacklistSize() const { return m_blacklist.size(); }

private:
	std::string computeSha256(const std::string& filePath);
	std::unordered_set<std::string> m_blacklist;
};