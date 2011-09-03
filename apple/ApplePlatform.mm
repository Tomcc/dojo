//
//  ApplePlatform.cpp
//  dojo
//
//  Created by Tommaso Checchi on 5/16/11.
//  Copyright 2011 none. All rights reserved.
//

#include "ApplePlatform.h"

#import <AudioToolbox/AudioToolbox.h>
#import <Foundation/NSLocale.h>

#ifdef PLATFORM_IOS
	#import <UIKit/UIApplication.h>
#endif

#include "Timer.h"
#include "Render.h"
#include "SoundManager.h"
#include "InputSystem.h"
#include "Game.h"
#include "Utils.h"
#include "Table.h"
#include "dojostring.h"
#include "StringReader.h"

using namespace Dojo;
using namespace std;

ApplePlatform::ApplePlatform( const Table& config ) :
Platform( config )
{
    pool = [[NSAutoreleasePool alloc] init];
	
	locale = String( [[NSLocale preferredLanguages ] objectAtIndex:0] );
}

ApplePlatform::~ApplePlatform()
{
	[pool release];
}

void ApplePlatform::step( float dt )
{
	Timer frameTimer;
	
    game->loop(dt);
    
    render->render();
    sound->update(dt);
	
	realFrameTime = frameTimer.getElapsedTime();
}

String ApplePlatform::getAppDataPath()
{
	NSArray* nspaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString* nspath = [nspaths objectAtIndex:0];
	
	return String( nspath );
}

String ApplePlatform::getRootPath()
{
	return String( [[NSBundle mainBundle] bundlePath] );
}

void ApplePlatform::loadPNGContent( void*& imageData, const String& path, uint& width, uint& height )
{
	width = height = 0;
	
	NSString* imgPath = path.toNSString();
		
	//magic OBJC code
	NSData *texData = [[NSData alloc] initWithContentsOfFile: imgPath ];
	
	DEBUG_ASSERT( texData );
	
	CGDataProviderRef prov = CGDataProviderCreateWithData( 
														  NULL, 
														  [texData bytes], 
														  [texData length], 
														  NULL );
	CGImageRef CGImage = CGImageCreateWithPNGDataProvider( prov, NULL, false, kCGRenderingIntentDefault );	
	
	width = (int)CGImageGetWidth(CGImage);
	height = (int)CGImageGetHeight(CGImage);	
	
	uint internalWidth = Math::nextPowerOfTwo( width );
	uint internalHeight = Math::nextPowerOfTwo( height );
	uint pitch = 4 * internalWidth;	
	
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	imageData = malloc( internalWidth * pitch );
	
	memset( imageData, 0, internalWidth * pitch );
	
	CGContextRef context = CGBitmapContextCreate(imageData, 
												 internalWidth, 
												 internalHeight, 
												 8, 
												 pitch, 
												 colorSpace, 
												 kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big );
	
	CGContextClearRect( context, CGRectMake( 0, 0, internalWidth, internalHeight ) );
	CGContextTranslateCTM( context, 0, internalHeight - height );
	CGContextDrawImage( context, CGRectMake( 0, 0, width, height ), CGImage );
	
	//correct premultiplied alpha - only in the useful part of the image
	byte* ptr = (byte*)imageData;
	byte* rowptr = (byte*)imageData;
	byte r, g, b;
	for( uint i = 0; i < height; ++i )
	{
		ptr = rowptr;
		
		for( uint j = 0; j < width; ++j )
		{
			byte alpha = ptr[3];
				
			if( alpha < 5 )
			{
				ptr[0] = r;
				ptr[1] = g;
				ptr[2] = b;
			}
			else if( alpha < 255 )
			{
				float a = ((float)(alpha)/255.f);
				
				r = (ptr[0] = ((float)ptr[0]) / a);
				g = (ptr[1] = ((float)ptr[1]) / a);
				b = (ptr[2] = ((float)ptr[2]) / a);
			}
			
			ptr += 4;
		}
		
		rowptr += pitch;
	}
	
	//free everything
	CGContextRelease(context);	
	CGColorSpaceRelease( colorSpace );
	CGImageRelease( CGImage );
	CGDataProviderRelease( prov );
	
	[texData release];
}

void ApplePlatform::_createApplicationDirectory()
{	
	String dirname = "/" + game->getName();
	
	NSArray* nspaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString* nspath = [nspaths objectAtIndex:0];
	NSString* nsgamedir = dirname.toNSString();	
	NSString* nsdir = [nspath stringByAppendingString:nsgamedir];
	
	//check if the directory exists, if not existing create it!
	NSFileManager *fileManager= [NSFileManager defaultManager]; 
	if(![ fileManager fileExistsAtPath:nsdir ])
	{
		bool success = [fileManager 
						createDirectoryAtPath:nsdir
						withIntermediateDirectories:YES 
						attributes:nil 
						error:NULL];
		
		DEBUG_ASSERT( success );
	}	
}

void ApplePlatform::openWebPage( const String& site )
{
	NSURL* url = [NSURL URLWithString: site.toNSString() ];
	
#ifdef PLATFORM_OSX
	[[NSWorkspace sharedWorkspace] openURL: url ];
#else
	[[UIApplication sharedApplication] openURL:url];
#endif
}