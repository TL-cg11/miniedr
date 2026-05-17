#include "YaraDetector.hpp"
#include "core/Logger.hpp"

YaraDetector::YaraDetector() {
	yr_initialize();
}

YaraDetector::~YaraDetector() {
	if (m_rules) {
		yr_rules_destroy(m_rules);
	}
	yr_finalize();
}

bool YaraDetector::loadRule(const std::string& rulePath) {
	YR_COMPILER* compiler = nullptr;
	int result = yr_compiler_create(&compiler);
	if (result != ERROR_SUCCESS) {
		Logger::error("yr_compiler_create failed");
		return false;
	}

	FILE* rules_file = NULL;
	fopen_s(&rules_file, rulePath.c_str(), "r");
	if (rules_file == NULL) {
		Logger::error("Failed to open rule file: " + rulePath);
		yr_compiler_destroy(compiler);
		return false;
	}

	result = yr_compiler_add_file(compiler, rules_file, NULL, rulePath.c_str());
	fclose(rules_file);
	if (result != ERROR_SUCCESS) {
		Logger::error("yr_compiler_add_file failed (compile errors in .yar file)");
		yr_compiler_destroy(compiler);
		return false;
	}

	result = yr_compiler_get_rules(compiler, &m_rules);
	if (result != ERROR_SUCCESS) {
		Logger::error("yr_compiler_get_rules failed");
		yr_compiler_destroy(compiler);
		return false;
	}

	yr_compiler_destroy(compiler);

	Logger::info("YARA rules loaded from: " + rulePath);
	return true;
}

ScanResult YaraDetector::scan(const std::string& filePath) {
	ScanResult result;
	result.detectorName = "YaraDetector";

	if (m_rules == nullptr) {
		Logger::error("YaraDetector::scan called before loadRule");
		return result;
	}

	int rc = yr_rules_scan_file(
		m_rules,
		filePath.c_str(),
		0,
		yaraCallback,
		&result,
		0
	);

	if (rc != ERROR_SUCCESS) {
		Logger::error("yr_rules_scan_file failed for: " + filePath);
	}

	return result;
}

int YaraDetector::yaraCallback(
	YR_SCAN_CONTEXT* ctx,
	int message,
	void* message_data,
	void* user_data
) {
	if (message != CALLBACK_MSG_RULE_MATCHING) {
		return CALLBACK_CONTINUE;
	}

	ScanResult* r = static_cast<ScanResult*>(user_data);

	YR_RULE* matched_rule = static_cast<YR_RULE*>(message_data);

	r->detected = true;
	r->ruleName = std::string("YaraRule:") + matched_rule->identifier;

	return CALLBACK_CONTINUE;
}