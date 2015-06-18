#pragma once

#include "dojo_common_header.h"

#ifdef __OBJC__
	#import <Foundation/NSString.h>
#endif

#define STRING_MAX_FLOAT_DIGITS 6

namespace Dojo {
#ifndef PLATFORM_WIN32
	typedef unsigned char byte;
#endif

	//define the right unichar
#ifndef __APPLE__
	typedef wchar_t unichar;
#else
    typedef unsigned short unichar;
#endif

	//define the unicode stuff
	typedef std::basic_stringstream<unichar> StringStream;

	typedef std::basic_string<unichar> _ustring;

	class String : public _ustring {
	public:

		static const String Empty;

		String() :
			_ustring() {

		}

		String(const String& s) :
			_ustring(s) {

		}

		String(const _ustring& s) :
			_ustring(s) {

		}

		String(char s) :
			_ustring() {
			append(1, (unichar)s);
		}

		String(unichar c) {
			append(1, c);
		}

		String(const char* s) :
			_ustring() {
			appendASCII(s);
		}

		//converts a string from UTF8
		String(const std::string& utf8) :
			_ustring() {
			appendUTF8(utf8);
		}

		String(int i, unichar paddingChar = 0) :
			_ustring() {
			appendInt(i, paddingChar);
		}

		String(float f) :
			_ustring() {
			appendFloat(f);
		}

		String(float f, byte digits) :
			_ustring() {
			appendFloat(f, digits);
		}

		///if found, replace the given substr with the given replacement - they can be of different lengths.
		void replaceToken(const String& substring, const String& replacement);

		size_t byteSize() {
			return size() * sizeof( unichar);
		}

		///converts this string into ASCII. WARNING: fails silently on unsupported chars!!!
		std::string ASCII() const;

		std::string UTF8() const;

		void appendASCII(const char* s);

		void appendUTF8(const std::string& utf8);

		void appendInt(int i, unichar paddingChar = 0);

		void appendFloat(float f, byte digits = 2);

		String toUpper();

		///appends raw data to this string. It has to be wchar_t bytes aligned!
		void appendRaw(const void* data, int sz);

#ifdef __OBJC__
		NSString* toNSString() const 
		{                       
			return [ NSString stringWithCharacters: data() length: size() ];
		}
		
		String( NSString* nss ) :
		_ustring()
		{
			appendNSString( nss );
		}
		
		String& appendNSString( NSString* nss )
		{
			DEBUG_ASSERT( nss, "NSString was null" );
			
			auto sz = size();
			resize( sz + [nss length] );
			
		///copy the whole string verbatim
			[nss getCharacters: (unichar*)data() + sz range: NSMakeRange( 0, [nss length] )];
			
			return *this;
		}
#endif

	};

	inline String operator+(const String& lhs, const String& rhs) {
		return String(lhs).append(rhs);
	}

	inline String operator+(const char* lhs, const String& rhs) {
		return String(lhs) + rhs;
	}

	inline String operator+(char lhs, const String& rhs) {
		return String(lhs) + rhs;
	}

	inline String operator+(const String& lhs, const char* rhs) {
		return lhs + String(rhs);
	}

	inline String operator+(const String& lhs, char rhs) {
		return lhs + String(rhs);
	}

}

namespace std {
	///hash specialization for unordered_maps
	template <>
	struct hash<Dojo::String> : public hash<Dojo::_ustring> {

	};
}
