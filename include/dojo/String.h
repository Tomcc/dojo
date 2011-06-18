#ifndef String_h__
#define String_h__

#include "dojo_common_header.h"

namespace Dojo 
{
	typedef unsigned int uint;
	typedef unsigned char byte;
	typedef unsigned short unichar;

	//define the unicode stuff
	typedef std::basic_fstream< unichar > FileStream;
	typedef std::basic_istream< unichar > InputStream;
	typedef std::basic_ostream< unichar > OutputStream;

	typedef std::basic_string< unichar > _ustring;

	class String : public _ustring
	{
	public:

		static const String EMPTY;

		String() :
		_ustring()
		{

		}

		String( const String& s ) :
		_ustring( s )
		{

		}

		String( const _ustring& s ) :
		_ustring( s )
		{

		}

		String( char s ) :
		_ustring()
		{
			append( 1, (unichar)s );
		}

		String( const char * s ) :
		_ustring()
		{
			appendASCII( s );
		}

		//converts a string from UTF8
		String( const std::string& utf8 ) :
		_ustring()
		{
			appendUTF8( utf8 );
		}

		///converts this string into ASCII. WARNING: fails silently on unsupported chars!!!
		inline std::string ASCII() const 
		{
			std::string res;

			unichar c;
			for( uint i = 0; i < size(); ++i )
			{
				c = at(i);
				if( c <= 0xff )
					res += (char)c;
			}

			return res;
		}

		inline std::string UTF8() const 
		{
			//HACK!!!!! make a real parser!!!
			return ASCII();
		}

		inline void appendASCII( const char* s )
		{
			for( uint i = 0; s[i] != 0; ++i )
				append( 1, (unichar)s[i] );
		}

		inline void appendUTF8( const std::string& utf8 )
		{
			//TODO do it better
			appendASCII( utf8.c_str() );
		}
	};

	inline String operator+ ( const String lhs, const String& rhs)
	{
		return String( lhs ).append( rhs );
	}

	inline String operator+ (const char* lhs, const String& rhs)
	{
		return String( lhs ) + rhs;
	}

	inline String operator+ (char lhs, const String& rhs)
	{
		return String( lhs ) + rhs;
	}

	inline String operator+ (const String& lhs, const char* rhs)
	{
		return lhs + String( rhs );
	}

	inline String operator+ (const String& lhs, char rhs)
	{
		return lhs + String( rhs );
	}
}

#endif // String_h__