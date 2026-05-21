#pragma once

#include <string>
#include <filesystem>

class Quarantine {
public:
	explicit Quarantine(const std::filesystem::path& quarantineDir);

	~Quarantine() = default;

	Quarantine(const Quarantine&) = delete;
	Quarantine& operator=(const Quarantine&) = delete;

	std::string quarantineFile(const std::string& originalPath,
		const std::string& ruleName);

private:
	std::filesystem::path m_quarantineDir;
};