#include "HashDetector.hpp"
#include "core/Logger.hpp"
#include <windows.h>
#include <bcrypt.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>	// std::transform
#include <cctype>		// std::tolower

bool HashDetector::loadBlacklist(const std::string& blacklistPath) {
	std::ifstream file(blacklistPath);
	if (!file) {
		Logger::error("Failed to open blacklist: " + blacklistPath);
		return false;
	}

	std::string line;
	while (std::getline(file, line)) {
		// 앞뒤 공백 및 문자 제거
		while (!line.empty() &&
			(line.front() == ' ' || line.front() == '\t' ||
				line.front() == '\r' || line.front() == '\n')) {
			line.erase(line.begin());
		}
		while (!line.empty() &&
			(line.back() == ' ' || line.back() == '\t' ||
				line.back() == '\r' || line.back() == '\n')) {
			line.pop_back();
		}

		// 빈 줄 스킵
		if (line.empty()) {
			continue;
		}

		// 소문자로 통일
		std::transform(line.begin(), line.end(), line.begin(),
			[](unsigned char c) {return std::tolower(c); });

		// 블랙리스트에 추가
		m_blacklist.insert(line);
	}

	Logger::info("Blacklist size: " + std::to_string(m_blacklist.size()));

	return true;
}

ScanResult HashDetector::scan(const std::string& filePath) {
	ScanResult result;
	result.detectorName = "HashDetector";
	
	// SHA256 계산
	std::string hash = computeSha256(filePath);
	if (hash.empty()) {
		return result;
	}

	// 블랙리스트 조회
	if (m_blacklist.count(hash) > 0) {
		result.detected = true;
		result.ruleName = "BlacklistHash:" + hash;
	}

	return result;
}

std::string HashDetector::computeSha256(const std::string& filePath) {
	BCRYPT_ALG_HANDLE hAlg = nullptr;
	BCRYPT_HASH_HANDLE hHash = nullptr;
	NTSTATUS status = 0;

	status = BCryptOpenAlgorithmProvider(
		&hAlg,
		BCRYPT_SHA256_ALGORITHM,
		nullptr,
		0
	);
	if (!BCRYPT_SUCCESS(status)) {
		return "";
	}

	// 몇 바이트가 필요한지 OS에게 받아오기
	DWORD cbHashObject = 0;
	DWORD cbResult = 0;
	status = BCryptGetProperty(
		hAlg,
		BCRYPT_OBJECT_LENGTH,
		(PUCHAR)&cbHashObject,
		sizeof(DWORD),
		&cbResult,
		0
	);
	if (!BCRYPT_SUCCESS(status)) {
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return "";
	}

	// 받아온 크기 만큼 메모리 할당
	std::vector<UCHAR> hashObject(cbHashObject);

	// 해시 객체 만들기
	status = BCryptCreateHash(
		hAlg,
		&hHash,
		hashObject.data(),
		cbHashObject,
		nullptr,
		0,
		0
	);
	if (!BCRYPT_SUCCESS(status)) {
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return "";
	}

	// 파일 열기 (바이너리 모드 필수)
	std::ifstream file(filePath, std::ios::binary);
	if (!file) {
		BCryptDestroyHash(hHash);
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return "";
	}

	// 1MB 청크 버퍼
	constexpr size_t CHUNK_SIZE = 1024 * 1024;
	std::vector<char> buffer(CHUNK_SIZE);

	// 청크 단위로 읽으면서 해시에 누적
	while (file) {
		file.read(buffer.data(), CHUNK_SIZE);
		std::streamsize bytesRead = file.gcount();

		if (bytesRead > 0) {
			status = BCryptHashData(
				hHash,
				(PUCHAR)buffer.data(),
				(ULONG)bytesRead,
				0
			);
			if (!BCRYPT_SUCCESS(status)) {
				BCryptDestroyHash(hHash);
				BCryptCloseAlgorithmProvider(hAlg, 0);
				return "";
			}
		}
	}

	// 해시 결과 그기 알아오기
	DWORD cbHashSize = 0;
	status = BCryptGetProperty(
		hAlg,
		BCRYPT_HASH_LENGTH,
		(PUCHAR)&cbHashSize,
		sizeof(DWORD),
		&cbResult,
		0
	);
	if (!BCRYPT_SUCCESS(status)) {
		BCryptDestroyHash(hHash);
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return "";
	}

	std::vector<UCHAR> hashBytes(cbHashSize);

	status = BCryptFinishHash(
		hHash,
		hashBytes.data(),
		cbHashSize,
		0
	);
	if (!BCRYPT_SUCCESS(status)) {
		BCryptDestroyHash(hHash);
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return "";
	}

	BCryptDestroyHash(hHash);
	BCryptCloseAlgorithmProvider(hAlg, 0);

	std::ostringstream oss;
	for (DWORD i = 0; i < cbHashSize; i++) {
		oss << std::hex
			<< std::setw(2)
			<< std::setfill('0')
			<< (int)hashBytes[i];
	}
	return oss.str();
}