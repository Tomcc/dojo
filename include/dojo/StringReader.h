#pragma once

#include "dojo_common_header.h"

#include "dojostring.h"

namespace Dojo
{
	///StringReader wraps a String to help parsing
	class StringReader
	{
	public:

		///UTF-32 constructor
		StringReader( const String& string );

		///UTF-8 constructor
		StringReader( const std::string& string );
		
		///returns a new unicode character or 0 if the stream ended
		unichar get();
		
		void back();
		
		//TODO move all to Utils
		static bool isNumber( unichar c );

		static bool isLowerCaseLetter( unichar c );
		
		static bool isUpperCaseLetter( unichar c );

		static bool isLetter( unichar c );

		///returns if the given char is ok for a name, 0-9A-Za-z
		static bool isNameCharacter( unichar c );
        
        static bool isHex( unichar c );

		static bool isWhiteSpace( unichar c );

		void skipWhiteSpace();
        
        byte getHexValue( unichar c );

		int getCurrentIndex();
        
        ///reads a formatted hex
        unsigned int readHex();
		
		float readFloat();

		///reads n raw bytes from the file
		void readBytes( void* dest, int sizeBytes );

	protected:

		const String* wcharStr;
		const std::string* utf8Str;

		size_t idx;
	};
}
