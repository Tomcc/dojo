#ifndef StringReader_h__
#define StringReader_h__

#include "dojo_common_header.h"

#include "dojostring.h"

namespace Dojo
{
	class StringReader
	{
	public:

		StringReader( const String& string ) :
		str( string ),
		  idx( 0 )
		{

		}

		inline bool eof()
		{
			return idx >= str.size();
		}
		
		inline unichar get()
		{
			return str[ idx++ ];
		}
		
		inline void back()
		{
			--idx;
		}
		
		inline static bool isNumber( unichar c )
		{
			return c >= '0' && c <= '9';
		}

		inline static bool isWhiteSpace( unichar c )
		{
			return c == ' ' || c == '\n' || c == '\r' || c == '\t';
		}

		inline void skipWhiteSpace()
		{
			while( !eof() && isWhiteSpace( get() ) );

			back(); //put back first non whitespace char
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
			while( state != PS_END && !eof() )
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
					else if( isWhiteSpace( c ) && count > 0 )
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
					if( isWhiteSpace(c) )
					{
						back();
						state = PS_END;
					}

					else if( isNumber(c) )
					{
						res += (float)(c-'0') / count;
						count *= 10.f;
					}
					else 
						state = PS_ERROR;
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

		inline void readBytes( void* dest, uint sz )
		{
			sz /= sizeof( unichar );

			//clamp into string
			if( idx + sz > str.size() )
				sz = (uint)str.size() - idx;

			byte* data = (byte*)str.data() + idx * sizeof( unichar );

			memcpy( dest, data, sz * sizeof( unichar ) );

			idx += sz;
		}

	protected:

		const String& str;
		uint idx;
	};
}
#endif // StringReader_h__