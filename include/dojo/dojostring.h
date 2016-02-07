#pragma once

namespace Dojo {
	class String {
	public:
		static const utf::string Empty;

		static bool isNumber(uint32_t c);

#ifdef WIN32
		static std::wstring toUTF16(const utf::string& str);

		static utf::string toUTF8(const std::wstring& wstr);
#endif
	};
}