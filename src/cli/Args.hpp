#pragma once

#include <string>
#include <optional>

struct Args {
	enum class Mode { Scan, Monitor };


	Mode mode;
	std::string target;

	// 옵션
	std::string rulesPath = "rules/test.yar";
	std::string hashesPath = "hashes.txt";
	std::string dbPath = "events.db";
	std::string quarantineDir = "quarantine";
	std::string jsonlPath = "events.jsonl";
};

std::optional<Args> parseArgs(int argc, char* argv[]);

void printHelp();