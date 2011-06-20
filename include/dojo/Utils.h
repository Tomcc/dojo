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

#include "dojo/dojo_common_header.h"

#include "dojo/Color.h"

#ifdef __OBJ__
#include <Foundation/NSString.h>
#endif

namespace Dojo 
{	
	class Utils
	{
	public:
				
		static int getLastOf( const String& str, unichar c )
		{			
			for( int i = (int)str.size()-1; i >= 0; --i )
			{
				if( str.at(i) == c )
					return i;
			}
			
			return -1;
		}
		
		inline static bool isNumber( unichar c )
		{
			return c >= '0' && c <= '9';
		}

		static uint toInt( const String& str, uint startPos = 0 )
		{
			uint num = 0; 
			char c;
			for( uint i = startPos; i < str.size(); ++i )
			{
				c = (char)str[i];
				
				if( isNumber( c ) ) //is it a number?
				{
					num *= 10;
					num += c-'0';	
				}
			}
			return num;
		}
				
		inline static String getFileExtension( const String& path )
		{
			String str;
			
			int dot = getLastOf( path, '.' );
			if( dot != -1 )
				str = path.substr( dot + 1 );
			
			return str;
		}

		static String getFileName( const String& str )
		{			
			size_t end = getLastOf( str, '.' );
			size_t start = getLastOf( str, '/' )+1;

			if( end < start ) //there isn't a file execption
				end = str.size();

			String res;
			for( size_t i = start; i < end; ++i )
				res += str.at(i);

			return res;
		}

		inline static String getDirectory( const String& str )
		{
			size_t end = getLastOf( str, '/' );

			if( end == -1 )
				return String::EMPTY;

			return str.substr( 0, end );
		}

		inline static bool hasExtension( const String& ext, const String& nameOrPath )
		{
			return (nameOrPath.size() > ext.size()) && (ext == nameOrPath.substr( nameOrPath.size() - ext.size() ));
		}
		
		inline static int getTagIdx( const String& str )
		{			
			int tagIdx = (int)str.size()-1;
			unichar c;
			for( ; tagIdx >= 0; --tagIdx )
			{
				c = str[ tagIdx ];
				
				if( c == '_' )
					return tagIdx;
					
				else if( !isNumber( c ) )
					break;   //if a non-number char is encountered, this was not a tag
			}
			
			return -1;
		}
		
		inline static int getTag( const String& str )
		{
			int tidx = getTagIdx( str );
			
			if( tidx != -1 )
				return toInt( str, getTagIdx( str ) );
			else
				return -1; //no tag
		}
				
		inline static String removeTag( String& str )
		{
			String res = str;
			
			uint tidx = getTagIdx( str );
			
			if( tidx != -1 )
				res.resize( tidx );  //tag found, remove
			
			return res;
		}
		
		inline static bool areStringsNearInSequence( const String& first, const String& second )
		{
			//get number postfixes
			int t1 = getTag( first );
			int t2 = getTag( second );
			
			return t1 >= 0 && t2 >= 0 && t1+1 == t2;
		}
	};	
}

#endif