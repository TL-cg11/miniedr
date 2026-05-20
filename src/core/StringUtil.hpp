#pragma once

#include <string>

namespace StringUtil {
	std::string wstringToUtf8(const std::wstring& w);
	std::wstring utf8ToWstring(const std::string& s);
}