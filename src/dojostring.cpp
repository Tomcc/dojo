#include "dojostring.h"

using Dojo::String;

bool String::isNumber(uint32_t c) {
	return c >= '0' and c <= '9';
}

#ifdef WIN32
#include "dojo_win_header.h"

std::wstring String::toUTF16(utf::string_view str) {
	if (str.empty()) {
		return std::wstring();
	}

	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.byte_size(), nullptr, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.byte_size(), &wstrTo[0], size_needed);
	return wstrTo;
}

utf::string String::toUTF8(const std::wstring& wstr) {
	if (wstr.empty()) {
		return utf::string();
	}

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), (LPSTR)strTo.data(), size_needed, nullptr, nullptr);

	return utf::string{ strTo.data() };
}
#endif


