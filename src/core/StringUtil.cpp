#include "StringUtil.hpp"
#include <Windows.h>

namespace StringUtil {
	std::string wstringToUtf8(const std::wstring& w) {
		if (w.empty()) return {};

		// 필요한 byte 수 미리 구하기
		int len = WideCharToMultiByte(
			CP_UTF8,             // UTF-8 로 변환
			0,                   // 플래그 X
			w.data(),            // 입력
			(int)w.size(),       // 입력 길이 (글자 수)
			nullptr, 0,          // 출력 NULL, 크기 0
			nullptr, nullptr
		);

		// 실제 변환
		std::string out(len, '\0');   // len 바이트짜리 문자열 미리 할당
		WideCharToMultiByte(
			CP_UTF8, 0,
			w.data(), (int)w.size(),
			out.data(), len,     // 출력 버퍼 + 크기
			nullptr, nullptr
		);

		return out;
	}

	std::wstring utf8ToWstring(const std::string& s) {
		if (s.empty()) return {};

		// 필요한 wchar 개수 구하기
		int wlen = MultiByteToWideChar(
			CP_UTF8, 0,
			s.data(),
			(int)s.size(),
			nullptr, 0
		);

		// 실제 변환
		std::wstring out(wlen, L'\0');
		MultiByteToWideChar(
			CP_UTF8, 0,
			s.data(), (int)s.size(),
			out.data(), wlen
		);

		return out;
	}
}