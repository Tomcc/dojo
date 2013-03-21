#ifndef StringReader_h__
#define StringReader_h__

#include "dojo_common_header.h"

#include "dojostring.h"

namespace Dojo
{
	///StringReader wraps a Dojo::String to help parsing
	class StringReader
	{
	public:

		///UTF-32 constructor
		StringReader( const String& string ) :
		wcharStr( &string ),
		utf8Str( NULL ),
		idx( 0 )
		{

		}

		///UTF-8 constructor
		StringReader( const std::string& string ) :
		utf8Str( &string ),
		wcharStr( NULL ),
		idx( 0 )
		{

		}
		
		///returns a new unicode character or 0 if the stream ended
		inline unichar get()
		{
            DEBUG_ASSERT( (wcharStr && !utf8Str) || (!wcharStr && utf8Str), "StringReader is uninitialized" );

			//HACK this doesn't care about utf8 multichars!
			if( (wcharStr && idx >= wcharStr->size()) || (utf8Str && idx >= utf8Str->size()) )
			{
				++idx;
				return 0;
			}
			else if( wcharStr )
			{
				return (*wcharStr)[ idx++ ];
			}
			else
			{
				return (unichar)(*utf8Str)[ idx++ ];
			}
		}
		
		inline void back()
		{
            DEBUG_ASSERT( idx > 0, "back: The StringReader is already at the start of the stream" );
            
			--idx;
		}
		
		inline static bool isNumber( unichar c )
		{
			return c >= '0' && c <= '9';
		}

		inline static bool isLowerCaseLetter( unichar c )
		{
			return c >= 'a' && c <= 'z';
		}
		
		inline static bool isUpperCaseLetter( unichar c )
		{
			return c >= 'A' && c <= 'Z';
		}

		inline static bool isLetter( unichar c )
		{
			return isLowerCaseLetter(c) || isUpperCaseLetter( c );
		}

		///returns if the given char is ok for a name, 0-9A-Za-z
		inline static bool isNameCharacter( unichar c )
		{
			return isNumber( c ) || isLetter( c );
		}
        
        inline static bool isHex( unichar c )
        {
            return isNumber( c ) || ( c >= 'a' && c <= 'f' );
        }

		inline static bool isWhiteSpace( unichar c )
		{
			return c == ' ' || c == '\n' || c == '\r' || c == '\t';
		}

		inline void skipWhiteSpace()
		{
			while( isWhiteSpace( get() ) );

			back(); //put back first non whitespace char
		}
        
        inline byte getHexValue( unichar c )
        {
            if( isNumber( c ) )
                return c - '0';
            else if( isHex( c ) )
                return 10 + c - 'a';
            else
            {
                DEBUG_FAIL( "WRONG HEX VALUE" );
                return 0;
            }
        }

		inline int getCurrentIndex()
		{
			return idx;
		}
        
        ///reads a formatted hex
        unsigned int readHex()
        {
            //skip 0x
            get();
            get();
            
            unsigned int n = 0;
                        
            //read exactly 8 digits - could crash if not enough are available
            for( int i = 0; i < 8; ++i )
                n += getHexValue( get() ) * (1 << ((7-i)*4));
                       
            return n;
        }
		
		float readFloat()
		{
			enum ParseState
			{
				PS_SIGN,
				PS_INT,
				PS_MANTISSA,
				PS_END,
				PS_ERROR
			} state = PS_SIGN;

			skipWhiteSpace();

			unichar c;
			float sign = 1;
			float count = 0;
			float res = 0;
			while( state != PS_END )
			{
				c = get();

				if( state == PS_SIGN )
				{
					if( c == '-' )		
						sign = -1;
					else if( isNumber( c ) )
					{
						back();

						state = PS_INT;
					}
					else if( !isWhiteSpace(c) )
						state = PS_ERROR;
				}
				else if( state == PS_INT )
				{
					if( c == '.' )
					{						
						state = PS_MANTISSA;
						count = 9 ;
					}

					else if( isNumber( c ) )
					{
						res *= 10;
						res += c - '0';
					}
					else if( count > 0 )
					{
						back();
						state = PS_END;
					}
					else //not enough digits
						state = PS_ERROR;	

					++count;
				}
				else if( state == PS_MANTISSA )
				{
					if( isNumber(c) )
					{
						res += (float)(c-'0') / count;
						count *= 10.f;
					}
					else //the number ended
					{
						back();
						state = PS_END;
					}
				}
				else if( state == PS_ERROR )
				{		
					//TODO do something for errors
					res = 0; //return 0
					state = PS_END;

				}			
			}
			
			return sign * res;
		}

		///reads n raw bytes from the file
		inline void readBytes( void* dest, int sizeBytes )
		{
			DEBUG_ASSERT( (wcharStr && !utf8Str) || (!wcharStr && utf8Str), "Reading bytes from an uninitialized StringReader" );

			//load format data
			int elemSize = wcharStr ? sizeof( unichar ) : 1;
			byte * buf = wcharStr ? (byte*)wcharStr->data() : (byte*)utf8Str->data();
			int startingByte = idx * elemSize;
			buf += startingByte; //go to current element
			int size = (wcharStr ? wcharStr->size() : utf8Str->size()) * elemSize;

			//clamp into string
			if( startingByte + sizeBytes > size  )
				sizeBytes = size - startingByte;

			memcpy( dest, buf, sizeBytes );

			idx += sizeBytes / elemSize;
		}

	protected:

		bool mEOF;

		const String* wcharStr;
		const std::string* utf8Str;

		uint idx;
	};
}
#endif // StringReader_h__