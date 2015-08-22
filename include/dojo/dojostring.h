#pragma once

#include "dojo_common_header.h"

#ifdef __OBJC__
	#import <Foundation/NSString.h>
#endif

#define STRING_MAX_FLOAT_DIGITS 6

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