/*
 *  WrapperUtils.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/25/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef WrapperUtils_h__
#define WrapperUtils_h__

#include "dojo_common_header.h"

#include "Color.h"

namespace Dojo {
	
	class Utils
	{
	public:
		
		static uint toNumericVersion( const std::string& versionString )
		{
			DEBUG_ASSERT( versionString.size() );
			char c;
			uint numericVersion = 0;
			for( uint i = 0; i < versionString.size(); ++i )
			{
				c = versionString[i];
				
				DEBUG_ASSERT( isNumber( c ) || c == '.' );
				
				if( c == '.' )
					numericVersion *= 10;
				else					
					numericVersion += (char)c - '0';
			}
			if( versionString.size() < 4 ) //major version only, "4.0", but we need 400
				numericVersion *= 10;
			
			return numericVersion;
		}
		
		static std::string getIOSVersionString()
		{
			return toSTDString( [[UIDevice currentDevice] systemVersion ] );
		}
				
		static uint getIOSVersion()
		{
			return toNumericVersion( getIOSVersionString() );
		}
		
		static bool isIOSVersionAtLeast( const std::string& ver )
		{			
			return  getIOSVersion() >= toNumericVersion( ver );
		}
		
		static NSString* toNSString( const std::string& str )
		{			
			uint sz = str.size();
			unichar* unic = (unichar*)malloc( sz * sizeof( unichar ) );
			for( uint i = 0; i < sz; ++i )
				unic[i] = (unichar)( str.at(i) );
						
			NSString* nstring = [[NSString alloc] initWithCharacters:(const unichar*)unic length:(NSUInteger)sz];
						
			free( unic );
			
			return nstring;			
		}
		
		static std::string toSTDString( NSString* s )
		{
			DEBUG_ASSERT( s );
			
			std::string str;
			
			unichar c;
			for( uint i = 0; i < [s length]; ++i )
			{
				c = [s characterAtIndex:i];
				str += (char)c;
			}
			
			return str;
		}
		
		inline static UIColor* toUIColor( const Color& c )
		{
			return [UIColor colorWithRed:c.r green:c.g blue:c.b alpha:c.a];
		}
		
		static int getLastOf( const std::string& str, char c )
		{			
			for( int i = str.size()-1; i >= 0; --i )
			{
				if( str.at(i) == c )
					return i;
			}
			
			return -1;
		}
		
		///leaves only the filename in the string
		static std::string getFileName( const std::string& str )
		{			
			int end = getLastOf( str, '.' );
			int start = getLastOf( str, '/' )+1;
			
			if( end < start ) //there isn't a file execption
				end = str.size();
			
			std::string res;
			for( uint i = start; i < end; ++i )
				res += str.at(i);
			
			return res;
		}
		
		inline static bool isNumber( char c )
		{
			return c >= '0' && c <= '9';
		}
		
		inline static bool isWhiteSpace( char c )
		{
			return c == ' ' || c == '\n' || c == '\r' || c == '\t';
		}
		
		static uint toInt( const std::string& str, uint startPos = 0 )
		{
			uint num = 0; 
			char c;
			for( uint i = startPos; i < str.size(); ++i )
			{
				c = str.at(i);
				
				if( isNumber( c ) ) //is it a number?
				{
					num *= 10;
					num += c-'0';	
				}
			}
			return num;
		}
		
		inline static void skipWhiteSpace( char*& buf, const char* eof )
		{
			DEBUG_ASSERT( buf && eof );
			
			while( isWhiteSpace( *buf ) && buf < eof )
				++buf;
		}
		
		inline static void skipToken( char*& buf, const char* eof )
		{
			DEBUG_ASSERT( buf && eof );
			
			while( !isWhiteSpace(*buf) && buf < eof )
				++buf;
		}
		
		///extracts an uint from a stream until a delimiter is found
		static uint toInt( char*& buf, const char* eof )
		{		
			DEBUG_ASSERT( buf && eof );
			
			skipWhiteSpace( buf, eof );
			
			uint num = 0; 						
			while( isNumber( *buf ) && buf < eof )
			{
				num *= 10;
				num += *buf-'0';	
				
				++buf;
			}
			
			skipWhiteSpace( buf, eof );
			
			return num;
		}
		
		///extracts a float from a stream
		static float toFloat( char*& buf, const char* eof )
		{
			DEBUG_ASSERT( buf && eof );
			
			enum ParseState
			{
				PS_SIGN,
				PS_INT,
				PS_MANTISSA,
				PS_END,
				PS_ERROR
			} state = PS_SIGN;
						
			skipWhiteSpace( buf, eof );
			
			char c;
			float sign = 1;
			float count = 0;
			float res = 0;
			while( state != PS_END && buf < eof )
			{
				c = *buf++;
				
				if( state == PS_SIGN )
				{
					if( c == '-' )		
						sign = -1;
					else if( isNumber( c ) )
					{
						--buf;
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
						state = PS_END;
					else //not enough digits
						state = PS_ERROR;	
					
					++count;
				}
				else if( state == PS_MANTISSA )
				{
					if( isWhiteSpace(c) )
					{
						if( count > 0 )
							state = PS_END;
						else 
							state = PS_ERROR;
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
					res = 0; //return 0
					state = PS_END;
				}			
			}
			
			skipWhiteSpace( buf, eof );
			
			return sign * res;
		}
		
		static std::string toString( char*& buf, const char* eof )
		{
			DEBUG_ASSERT( buf && eof );
			
			skipWhiteSpace( buf, eof );
			
			std::string res;
			while( !isWhiteSpace( *buf ) && buf < eof ) 
				res += *buf++;
			
			return res;
		}
		
		static char* findFirstLineEnd( char* c, const char* eof )
		{		
			DEBUG_ASSERT( c && eof );
			
			for( ; *c != '\n' && c < eof; ++c )
			{
				
			}
			
			return c;
		}
		
		static uint streamsEqual( const char* s1, const char* s2 )
		{
			DEBUG_ASSERT( s1 && s2 );
			
			uint matches = 0;
			
			for( ; *s1 == *s2; ++s1, ++s2 )				
				++matches;
			
			return matches;
		}
		
		//checks if the string starting at "buf" contains "str"
		static bool tokenEquals( const char* buf, const char* str )
		{			
			DEBUG_ASSERT( buf && str );
			
			for( uint i = 0; str[i] != '\0'; ++i )
			{				
				if( buf[i] != str[i] )
					return false;
			}
			return true;
		}
		
		static void nextToken( char*& buf, const char* eof )
		{			
			skipWhiteSpace( buf, eof );
			skipToken( buf, eof );
			skipWhiteSpace( buf, eof );
		}
		
		static std::string toString( uint i, char paddingChar = 0 )
		{				
			std::string res;
			
			if( i == 0 )
			{
				for( uint p = 0; p < 9; ++p )
					res += paddingChar;
				
				return res + '0';
			}			
					
			bool nonZeroFound = false;
			char c;
			for( uint div = 1000000000; div > 0; div /= 10 )
			{
				c = '0' + (i / div);
				
				if( !nonZeroFound && c != '0' )
					nonZeroFound = true;
				
				if( nonZeroFound )	
					res += c;
				
				else if( paddingChar )
					res += paddingChar;
				
				i %= div;
			}
			
			
			return res;
		}
		
		inline static std::string getFileExtension( const std::string& path )
		{
			std::string str;
			
			int dot = getLastOf( path, '.' );
			if( dot != -1 )
				str = path.substr( dot + 1 );
			
			return str;
		}
		
		inline static int getTagIdx( const std::string& str )
		{			
			int tagIdx = str.size()-1;
			char c;
			for( ; tagIdx >= 0; --tagIdx )
			{
				c = str.at( tagIdx );
				
				if( c == '_' )
					return tagIdx;
					
				else if( !isNumber( c ) )
					break;   //if a non-number char is encountered, this was not a tag
			}
			
			return -1;
		}
		
		inline static int getTag( const std::string& str )
		{
			int tidx = getTagIdx( str );
			
			if( tidx != -1 )
				return toInt( str, getTagIdx( str ) );
			else
				return -1; //no tag
		}
				
		inline static std::string removeTag( std::string& str )
		{
			std::string res = str;
			
			uint tidx = getTagIdx( str );
			
			if( tidx != -1 )
				res.resize( tidx );  //tag found, remove
			
			return res;
		}
		
		inline static bool areStringsNearInSequence( const std::string& first, const std::string& second )
		{
			//get number postfixes
			uint t1 = getTag( first );
			uint t2 = getTag( second );
			
			return t1 > 0 && t1+1 == t2;
		}
	};	
}

#endif