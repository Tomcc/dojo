#ifndef String_h__
#define String_h__

#include "dojo_common_header.h"

#ifdef __OBJC__
	#import <Foundation/NSString.h>
#endif

#define STRING_MAX_FLOAT_DIGITS 6

namespace Dojo 
{
	typedef unsigned int uint;
	typedef unsigned char byte;
	typedef unsigned short unichar;

	//define the unicode stuff
	typedef std::basic_stringstream< unichar > StringStream;

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
		
		String( int i, unichar paddingChar = 0 ) :
		_ustring()
		{
			appendInt( i, paddingChar );
		}
		
		String( uint i, unichar paddingChar = 0 ) :
		_ustring()
		{
			appendInt( i, paddingChar );
		}
		
		String( float f ) :
		_ustring()
		{
			appendFloat( f );
		}
		
		inline size_t byteSize()
		{
			return size() * sizeof( unichar );
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
		
		void appendInt( int i, unichar paddingChar = 0 )
		{
			uint div = 1000000000;
			unichar c;
			
			for( ; div > 0; i %= div, div /= 10 )
			{
				c = '0' + (i / div);
				
				if( c != '0' )  break;
				else if( paddingChar )
					*this += paddingChar;
			}
			
			if( i == 0 )
				*this += '0';
			
			for( ; div > 0; i %= div, div /= 10 )
				*this += '0' + (i / div);
			
		}
		
		inline void appendFloat( float f )
		{
			//HACK
			std::stringstream str;
			str << f;
			
			appendASCII( str.str().c_str() );
		}
		
		///appends data to this string. It has to be 2 bytes aligned!
		inline void appendRaw( void* data, uint sz )
		{
			DEBUG_ASSERT( sz % sizeof( unichar ) == 0 );
			
			append( (unichar*)data, sz / sizeof( unichar ) );
		}
				
#ifdef __OBJC__
		inline NSString* toNSString() const 
		{                       
			return [ NSString stringWithCharacters: data() length: size() ];
		}
		
		String( NSString* nss ) :
		_ustring()
		{
			appendNSString( nss );
		}
		
		inline String& appendNSString( NSString* nss )
		{
			DEBUG_ASSERT( nss );
			
			uint sz = size();
			resize( sz + [nss length] );
			
			///copy the whole string verbatim
			[nss getCharacters: (unichar*)data() + sz range: NSMakeRange( 0, [nss length] )];
			
			return *this;
		}
#endif
		
	};
	
	inline String operator+ ( const String& lhs, const String& rhs)
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