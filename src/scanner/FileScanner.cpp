#include "FileScanner.hpp"
#include "core/Logger.hpp"
#include <filesystem>

namespace fs = std::filesystem;

std::vector<std::string> FileScanner::scan(const std::string& directory) {
	std::vector<std::string> result;

	try {
		for (const auto& entry : fs::recursive_directory_iterator(directory)) {
			if (entry.is_regular_file()) {
				result.push_back(entry.path().generic_string());
			}
		}
	}
	catch (const std::exception& e) {
		Logger::error("FileScanner failed to scan '" + directory + "': " + e.what());
	}

	return result;
}