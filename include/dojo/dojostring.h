#pragma once

#include "dojo_common_header.h"

#ifdef __OBJC__
	#import <Foundation/NSString.h>
#endif

#define STRING_MAX_FLOAT_DIGITS 6

namespace Dojo {
	class String {
	public:
		static const std::string Empty;

		static void append(std::string& str, uint32_t c);

		static bool isNumber(uint32_t c);

#ifdef WIN32
		static std::wstring toUTF16(const std::string& str);

		static std::string toUTF8(const std::wstring& wstr);
#endif
	};
}