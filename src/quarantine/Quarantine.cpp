#include "quarantine/Quarantine.hpp"
#include "core/Logger.hpp"

#include <Windows.h>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

Quarantine::Quarantine(const fs::path& quarantineDir) : m_quarantineDir(quarantineDir) {
	std::error_code ec;
	fs::create_directories(m_quarantineDir, ec);

	if (ec) {
		Logger::error("Quarantine: failed to create directory '" + m_quarantineDir.string() + "', err=" + ec.message());
	}
	else {
		Logger::info("Quarantine: ready at '" + m_quarantineDir.string() + "'");
	}
}

std::string Quarantine::quarantineFile(const std::string& originalPath, const std::string& ruleName) {

	fs::path srcPath = fs::u8path(originalPath);

	// 원본 파일 존재 확인
	std::error_code ec;
	if (!fs::exists(srcPath, ec)) {
		Logger::info("Quarantine: '" + originalPath + "' already gone (likely already quarantined). Skipping.");
		return "";
	}

	// 파일명 추출
	std::string originalFilename = srcPath.filename().u8string();

	// 타임스탬프 문자열
	auto now = std::chrono::system_clock::now();
	auto now_t = std::chrono::system_clock::to_time_t(now);
	std::tm tm_local{};
	localtime_s(&tm_local, &now_t);

	std::ostringstream tsStream;
	tsStream << std::put_time(&tm_local, "%Y%m%d_%H%M%S");
	std::string timestamp = tsStream.str();

	// 문자열 합치기
	std::string quarantineName = timestamp + "_" + originalFilename + ".qtn";

	// 전체 경로
	fs::path destPath = m_quarantineDir / fs::u8path(quarantineName);

	// UTF-16으로 변환
	std::wstring wSrc = srcPath.wstring();
	std::wstring wDest = destPath.wstring();

	BOOL ok = MoveFileExW(
		wSrc.c_str(),
		wDest.c_str(),
		MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH
	);
	if (!ok) {
		DWORD err = GetLastError();
		Logger::error("Quarantine: MoveFileExW failed for '" + originalPath + "', err=" + std::to_string(err));
		return "";
	}

	if (!fs::exists(destPath, ec)) {
		Logger::error("Quarantine: MoveFileExW returned success but '" + destPath.u8string() + "' does not exist");
		return "";
	}

	Logger::info("Quarantine: '" + originalPath + "' -> '" + destPath.u8string() + "' (rule=" + ruleName + ")");

	return destPath.u8string();
}