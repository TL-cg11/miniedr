#include "Args.hpp"
#include <iostream>
#include <cstring>

std::optional<Args> parseArgs(int argc, char* argv[]) {
	Args args;

	if (argc < 2) {
		printHelp();
		return std::nullopt;
	}

	std::string firstArg = argv[1];
	if (firstArg == "--help" || firstArg == "-h") {
		printHelp();
		return std::nullopt;
	}
	if (firstArg == "scan") {
		args.mode = Args::Mode::Scan;
	}
	else if (firstArg == "monitor") {
		args.mode = Args::Mode::Monitor;
	}
	else {
		std::cerr << "Unknown mode: " << firstArg << "\n";
		printHelp();
		return std::nullopt;
	}

	if (argc < 3) {
		std::cerr << "Missing target directory\n";
		printHelp();
		return std::nullopt;
	}

	std::string targetArg = argv[2];
	if (!targetArg.empty() && targetArg[0] == '-') {
		std::cerr << "Missing target directory (got option '" << targetArg << "' instead)\n";
		printHelp();
		return std::nullopt;
	}
	args.target = targetArg;

	for (int i = 3; i < argc; i++) {
		std::string opt = argv[i];

		if (opt == "--rules")	{
			if (i + 1 >= argc) {
				std::cerr << "Missing value for " << opt << "\n";
				return std::nullopt;
			}
			args.rulesPath = argv[i + 1];
			i++;
		}
		else if (opt == "--hashes") {
			if (i + 1 >= argc) {
				std::cerr << "Missing value for " << opt << "\n";
				return std::nullopt;
			}
			args.hashesPath = argv[i + 1];
			i++;
		}
		else if (opt == "--db") {
			if (i + 1 >= argc) {
				std::cerr << "Missing value for " << opt << "\n";
				return std::nullopt;
			}
			args.dbPath = argv[i + 1];
			i++;
		}
		else if (opt == "--quarantine") {
			if (i + 1 >= argc) {
				std::cerr << "Missing value for " << opt << "\n";
				return std::nullopt;
			}
			args.quarantineDir = argv[i + 1];
			i++;
		}
		else if (opt == "--jsonl") {
			if (i + 1 >= argc) {
				std::cerr << "Missing value for " << opt << "\n";
				return std::nullopt;
			}
			args.jsonlPath = argv[i + 1];
			i++;
		}
		else {
			std::cerr << "Unknown option: " << opt << "\n";
			printHelp();
			return std::nullopt;
		}
	}

	return args;
}

void printHelp() {
	std::cerr <<
		"\n"
		"miniedr - Mini Endpoint Detection and Response\n"
		"\n"
		"Usage:\n"
		"  miniedr <mode> <target> [options]\n"
		"  miniedr --help | -h\n"
		"\n"
		"Modes:\n"
		"  scan       Scan a directory once and exit\n"
		"  monitor    Watch a directory in real-time until 'stop' is entered\n"
		"\n"
		"Options:\n"
		"  --rules <path>       YARA rule file       (default: rules/test.yar)\n"
		"  --hashes <path>      Hash blacklist file  (default: hashes.txt)\n"
		"  --db <path>          SQLite database      (default: events.db)\n"
		"  --quarantine <path>  Quarantine directory (default: quarantine)\n"
		"  --jsonl <path>       JSON Lines log file  (default: events.jsonl)\n"
		"\n"
		"Examples:\n"
		"  miniedr scan C:/EDR-Test/watch\n"
		"  miniedr monitor C:/EDR-Test/watch --rules custom.yar\n"
		"  miniedr scan C:/test --db prod.db --jsonl prod.jsonl\n"
		"\n";
}