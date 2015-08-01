#include "dojostring.h"

using namespace Dojo;

bool String::isNumber(uint32_t c) {
	return c >= '0' && c <= '9';
}

#ifdef WIN32
std::wstring String::toUTF16(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

std::string String::toUTF8(const std::wstring& wstr) {
	if (wstr.empty()) {
		return std::string();
	}

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
	return strTo;
}
#endif