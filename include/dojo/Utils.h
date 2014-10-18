/*
 *  WrapperUtils.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/25/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "Color.h"

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
		
		static bool isNumber( unichar c )
		{
			return c >= '0' && c <= '9';
		}

		static int toInt( const String& str, int startPos = 0 )
		{
			int num = 0; 
			unichar c;
			for( size_t i = startPos; i < str.size(); ++i )
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
				
		static String getFileExtension( const String& path )
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

			if( end < start ) //there isn't a file extension
				end = str.size();

			String res;
			for( size_t i = start; i < end; ++i )
				res += str.at(i);

			return res;
		}

		static String getDirectory( const String& str )
		{
			int end = getLastOf( str, '/' );

			if( end == -1 )
				return String::EMPTY;

			return str.substr( 0, end );
		}

		static bool isAbsolutePath(const String& str) {
			return str[1] == ':' || str[0] == '/';
		}
		
		///replace any "\\" in the path with the canonical / and removes any last /
		static void makeCanonicalPath( String& path )
		{			
			for( size_t i = 0; i < path.size(); ++i )
			{
				if( path[i] == '\\' )
					path[i] = '/';
			}
			
			//remove ending /
			if( path[path.size()-1] == '/')
				path.resize( path.size()-1 );
		}

		static bool hasExtension( const String& ext, const String& nameOrPath )
		{
			return (nameOrPath.size() > ext.size()) && (ext == nameOrPath.substr( nameOrPath.size() - ext.size() ));
		}
		
		static int getTagIdx( const String& str )
		{			
			int tagIdx = getVersionIdx( str )-1; //get version idx
			
			if( tagIdx < 0 )
				tagIdx = (int)str.size()-1;
			
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
		
		static int getVersionIdx( const String& str )
		{            
			int idx = (int)str.size()-1;
			
			//look for a single digit
			for( ; idx >= 0 && !isNumber( str.at( idx ) ); --idx );
			
			return ( idx > 1 && str.at( idx-1 ) == '@' ) ? idx-1 : -1;
		}
		
		///returns the version of the given name, or 0 if not found
		static int getVersion( const String& str )
		{
			int vidx = getVersionIdx( str );
			
			return (vidx >= 0) ? (str.at( vidx+1 ) - '0') : 0;
		}
		
		static int getTag( const String& str )
		{
			int tidx = getTagIdx( str );
			int end = getVersionIdx( str );
			
			if( end == -1 )
				end = str.size();
			
			if( tidx != -1 )
				return toInt( str.substr( tidx+1, end - tidx - 1 ) );
			else
				return -1; //no tag
		}
						
		///removes extra info appended to file name: "filename_3@2" -> "filename"
		static String removeTags( const String& str )
		{            
			int tidx = getTagIdx( str );
			
			//if a tag is found, just remove everything after
			if( tidx != -1 )
				return str.substr( 0, tidx );
			
			int vidx = getVersionIdx( str );
			if( vidx != -1 ) //else remove just the version
				return str.substr( 0, vidx );
			
			return str;
		}

		static String removeVersion( const String& str )
		{
			int vidx = getVersionIdx( str );
			if( vidx != -1 ) //else remove just the version
				return str.substr( 0, vidx );

			return str;
		}
		
		static bool areStringsNearInSequence( const String& first, const String& second )
		{
			//get number postfixes
			int t1 = getTag( first );
			int t2 = getTag( second );
			
			return t1 >= 0 && t2 >= 0 && t1+1 == t2;
		}
		
		static void swap( float& a, float& b )
		{
			float temp = a;
			a = b;
			b = temp;
		}
	};	
}

