#include "FileScanner.hpp"
#include "core/Logger.hpp"
#include <filesystem>

namespace fs = std::filesystem;

std::vector<std::string> FileScanner::scan(const std::string& directory) {
	std::vector<std::string> result;

	std::error_code ec;
	if (!fs::exists(directory, ec)) {
		Logger::error("FileScanner: directory does not exist: " + directory);
		return result;
	}
	if (!fs::is_directory(directory, ec)) {
		Logger::error("FileScanner: not a directory: " + directory);
		return result;
	}

	try {
		for (const auto& entry : fs::recursive_directory_iterator(directory, fs::directory_options::skip_permission_denied)) {
			if (entry.is_regular_file()) {
				result.push_back(entry.path().generic_u8string());
			}
		}
	}
	catch (const std::exception& e) {
		Logger::error("FileScanner failed to scan '" + directory + "': " + e.what());
	}

	return result;
}