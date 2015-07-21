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

		static std::string fromInt(int i);
		static std::string fromFloat(float f, int precision = 2);

		static int toInt(const std::string& str, int startPos = 0);

		static void append(std::string& str, uint32_t c);

		static bool isNumber(uint32_t c);

		static int getLastOf(const std::string& str, uint32_t c);
	};

	class Path {
		public:
		static std::string getFileExtension(const std::string& path);

		static std::string getFileName(const std::string& str);

		static std::string getDirectory(const std::string& str);

		static bool isAbsolute(const std::string& str);

		///replace any "\\" in the path with the canonical / and removes any last /
		static void makeCanonical(std::string& path);

		static bool hasExtension(const std::string& ext, const std::string& nameOrPath);

		static int getTagIdx(const std::string& str);

		static int getVersionIdx(const std::string& str);

		///returns the version of the given name, or 0 if not found
		static int getVersion(const std::string& str);

		static int getTag(const std::string& str);

		///removes extra info appended to file name: "filename_3@2" -> "filename"
		static std::string removeTags(const std::string& str);

		static std::string removeVersion(const std::string& str);

		static bool arePathsInSequence(const std::string& first, const std::string& second);
	};
// #ifndef PLATFORM_WIN32
// 	typedef unsigned char byte;
// #endif
// 
// 	//TODO switch to UTF8 ffs this is horrible!
// 	//define the right uint32_t
// #ifndef __APPLE__
// 	typedef wchar_t uint32_t;
// #else
//     typedef unsigned short uint32_t;
// #endif
// 
// 	//define the unicode stuff
// 	typedef std::basic_stringstream<uint32_t> StringStream;
// 
// 	typedef std::basic_string<uint32_t> _ustring;
// 
// 	class std::string : public _ustring {
// 	public:
// 
// 		static const std::string Empty;
// 
// 		std::string() :
// 			_ustring() {
// 
// 		}
// 
// 		std::string(const std::string& s) :
// 			_ustring(s) {
// 
// 		}
// 
// 		std::string(const _ustring& s) :
// 			_ustring(s) {
// 
// 		}
// 
// 		std::string(char s) :
// 			_ustring() {
// 			append(1, (uint32_t)s);
// 		}
// 
// 		std::string(uint32_t c) {
// 			append(1, c);
// 		}
// 
// 		std::string(const char* s) :
// 			_ustring() {
// 			appendASCII(s);
// 		}
// 
// 		//converts a string from UTF8
// 		std::string(const std::string& utf8) :
// 			_ustring() {
// 			appendUTF8(utf8);
// 		}
// 
// 		std::string(int i, uint32_t paddingChar = 0) :
// 			_ustring() {
// 			appendInt(i, paddingChar);
// 		}
// 
// 		std::string(float f) :
// 			_ustring() {
// 			appendFloat(f);
// 		}
// 
// 		std::string(float f, byte digits) :
// 			_ustring() {
// 			appendFloat(f, digits);
// 		}
// 
// 		///if found, replace the given substr with the given replacement - they can be of different lengths.
// 		void replaceToken(const std::string& substring, const std::string& replacement);
// 
// 		size_t byteSize() {
// 			return size() * sizeof( uint32_t);
// 		}
// 
// 		///converts this string into ASCII. WARNING: fails silently on unsupported chars!!!
// 		std::string ASCII() const;
// 
// 		std::string UTF8() const;
// 
// 		void appendASCII(const char* s);
// 
// 		void appendUTF8(const std::string& utf8);
// 
// 		void appendInt(int i, uint32_t paddingChar = 0);
// 
// 		void appendFloat(float f, byte digits = 2);
// 
// 		std::string toUpper();
// 
// 		///appends raw data to this string. It has to be wchar_t bytes aligned!
// 		void appendRaw(const void* data, int sz);
// 
// #ifdef __OBJC__
// 		NSString* toNSString() const 
// 		{                       
// 			return [ NSString stringWithCharacters: data() length: size() ];
// 		}
// 		
// 		std::string( NSString* nss ) :
// 		_ustring()
// 		{
// 			appendNSString( nss );
// 		}
// 		
// 		std::string& appendNSString( NSString* nss )
// 		{
// 			DEBUG_ASSERT( nss, "NSString was null" );
// 			
// 			auto sz = size();
// 			resize( sz + [nss length] );
// 			
// 		///copy the whole string verbatim
// 			[nss getCharacters: (uint32_t*)data() + sz range: NSMakeRange( 0, [nss length] )];
// 			
// 			return *this;
// 		}
// #endif
// 
// 	};
// 
// 	inline std::string operator+(const std::string& lhs, const std::string& rhs) {
// 		return std::string(lhs).append(rhs);
// 	}
// 
// 	inline std::string operator+(const char* lhs, const std::string& rhs) {
// 		return std::string(lhs) + rhs;
// 	}
// 
// 	inline std::string operator+(char lhs, const std::string& rhs) {
// 		return std::string(lhs) + rhs;
// 	}
// 
// 	inline std::string operator+(const std::string& lhs, const char* rhs) {
// 		return lhs + std::string(rhs);
// 	}
// 
// 	inline std::string operator+(const std::string& lhs, char rhs) {
// 		return lhs + std::string(rhs);
// 	}
};

// namespace std {
// 	///hash specialization for unordered_maps
// 	template <>
// 	struct hash<std::string> : public hash<Dojo::_ustring> {
// 
// 	};
// }
