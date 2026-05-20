#include "YaraDetector.hpp"
#include "core/Logger.hpp"
#include "core/StringUtil.hpp"
#include <Windows.h>
#include <vector>

namespace {
	std::vector<BYTE> readFileToMemory(const std::wstring& wpath) {
		HANDLE h = CreateFileW(
			wpath.c_str(),
			GENERIC_READ,			// 읽기 권한
			FILE_SHARE_READ,		// 다른 프로세스 읽기 허용
			NULL,
			OPEN_EXISTING,			// 기존 파일만
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if (h == INVALID_HANDLE_VALUE) {
			return {};
		}

		// 파일 크기 확인
		LARGE_INTEGER size;
		if (!GetFileSizeEx(h, &size)) {
			CloseHandle(h);
			return {};
		}

		// 비정상 크기 처리
		const LONGLONG MAX_SIZE = 100LL * 1024 * 1024;	// 100MB
		if (size.QuadPart == 0 || size.QuadPart > MAX_SIZE) {
			CloseHandle(h);
			return {};
		}

		// 메모리 할당
		std::vector<BYTE> buffer((size_t)size.QuadPart);

		// 파일 내용 읽기
		DWORD bytesRead = 0;
		BOOL ok = ReadFile(
			h,
			buffer.data(),
			(DWORD)buffer.size(),
			&bytesRead,
			NULL
		);
		CloseHandle(h);

		if (!ok || bytesRead != buffer.size()) {
			return {};
		}

		return buffer;
	}
}

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

ScanResult YaraDetector::scan(const std::string& path) {
	ScanResult result;
	result.detectorName = "YaraDetector";

	std::wstring wpath = StringUtil::utf8ToWstring(path);

	std::vector<BYTE> bytes = readFileToMemory(wpath);
	if (bytes.empty()) {
		Logger::info("YARA scan skipped (empty or unreadable): " + path);
		return result;
	}

	int rc = yr_rules_scan_mem(
		m_rules,
		bytes.data(),
		bytes.size(),
		0,
		yaraCallback,
		&result,
		0
	);

	if (rc != ERROR_SUCCESS) {
		Logger::error("yr_rules_scan_file failed for: " + path);
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