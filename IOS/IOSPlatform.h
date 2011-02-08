#ifndef IOSPlatform_h__
#define IOSPlatform_h__

#include "dojo_common_header.h"

namespace Dojo
{
	class IOSPlatform : public Platform
	{
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
	};
}

#endif