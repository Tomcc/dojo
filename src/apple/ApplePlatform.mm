//
//  ApplePlatform.cpp
//  dojo
//
//  Created by Tommaso Checchi on 5/16/11.
//  Copyright 2011 none. All rights reserved.
//

#include "ApplePlatform.h"

#import <CoreFoundation/CoreFoundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <Foundation/NSLocale.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSFileManager.h>

#ifdef PLATFORM_OSX
	#import <ApplicationServices/ApplicationServices.h>
#else
	#import <CoreGraphics/CoreGraphics.h>
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
	//HACK
    //pool = [[NSAutoreleasePool alloc] init];

	if( [[NSLocale preferredLanguages] count ] )
		locale = String( [[NSLocale preferredLanguages ] objectAtIndex:0] );
	else
		locale = "en";
}

ApplePlatform::~ApplePlatform()
{
	//HACK
	//[pool release];
}

void ApplePlatform::step( float dt )
{
	Timer frameTimer;
	
	//clamp to max dt to avoid crazy behaviour
	dt = Math::min( dt, game->getMaximumFrameLength() );		
	
    game->loop(dt);
    
    render->render();
    sound->update(dt);
	
	realFrameTime = frameTimer.getElapsedTime();
}

bool ApplePlatform::loadPNGContent( void*& imageData, const String& path, int& width, int& height )
{
	width = height = 0;
		
	NSURL* url = [NSURL fileURLWithPath: path.toNSString() ];
	
	CGDataProviderRef prov = CGDataProviderCreateWithURL( (CFURLRef)url );
	
	if( prov == nil )
		return false;
	
	CGImageRef CGImage = CGImageCreateWithPNGDataProvider( prov, NULL, true, kCGRenderingIntentDefault );
		
	width = (int)CGImageGetWidth(CGImage);
	height = (int)CGImageGetHeight(CGImage);	
	
	int pitch = 4 * width;
	int size = pitch * height;
	
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

	imageData = malloc( size);
	memset( imageData, 0, size );
	
	CGContextRef context = CGBitmapContextCreate(imageData,
												 width,
												 height,
												 8,
												 pitch,
												 colorSpace,
												 kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big );
	
	CGContextDrawImage( context, CGRectMake( 0, 0, width, height ), CGImage );
	
	//correct premultiplied alpha - only in the useful part of the image
	///while doing this, extend the color to the pitch row to avoid artifacts at borders
	byte* ptr;
	byte* rowptr = (byte*)imageData;
	byte r = 0, g = 0, b = 0;
	
	for( int i = 0; i < height; ++i )
	{
		ptr = rowptr;
				
		for( int j = 0; j < width; ++j )
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

	CGDataProviderRelease( prov );
	CGImageRelease( CGImage );
	//[image release];
	
	return true;
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
