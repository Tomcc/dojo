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
#include "BackgroundQueue.h"

using namespace Dojo;
using namespace std;

ApplePlatform::ApplePlatform( const Table& config ) :
Platform( config )
{
    pool = [[NSAutoreleasePool alloc] init];

	if( [[NSLocale preferredLanguages] count ] )
		locale = String( [[NSLocale preferredLanguages ] objectAtIndex:0] );
	else
		locale = "en";
    
    
    //find logic cpu cores count
    mCPUCores = sysconf( _SC_NPROCESSORS_ONLN );
}

ApplePlatform::~ApplePlatform()
{
	[pool release];
}

void ApplePlatform::step( float dt )
{
    frameTimer.reset();
	
	//clamp to max dt to avoid crazy behaviour
	dt = Math::min( dt, game->getMaximumFrameLength() );
    
    mBackgroundQueue->fireCompletedCallbacks();
	
    input->poll( dt );
	
    game->loop(dt);
    
    sound->update(dt);
    
    render->render();
}

GLenum ApplePlatform::loadImageFile( void*& bufptr, const String& path, int& width, int& height, int& pixelSize )
{
    width = height = 0;
    
	NSURL* url = [NSURL fileURLWithPath: path.toNSString() ];
	
	CGDataProviderRef prov = CGDataProviderCreateWithURL( (CFURLRef)url );
	
	if( prov == nil )
		return 0;
	
	CGImageRef CGImage;
    
    String ext = Utils::getFileExtension( path );
    
    if( ext == String( "png" ) )
        CGImage = CGImageCreateWithPNGDataProvider( prov, NULL, true, kCGRenderingIntentDefault );
    else if( ext == String( "jpg" ) )
        CGImage = CGImageCreateWithJPEGDataProvider( prov, NULL, true, kCGRenderingIntentDefault );
    else
    {
        DEBUG_TODO;
    }
    
    pixelSize = (int)CGImageGetBitsPerPixel( CGImage ) / 8;
    bool alphaChannel = pixelSize == 4;
    
	width = (int)CGImageGetWidth(CGImage);
	height = (int)CGImageGetHeight(CGImage);
	
	int pitch = pixelSize * width;
	int size = pitch * height;
	
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    
	bufptr = malloc( size );
	memset( bufptr, 0, size );
    
    CFDataRef data = CGDataProviderCopyData(CGImageGetDataProvider(CGImage));
    const unsigned char * buffer =  CFDataGetBytePtr(data);
    memcpy( bufptr, buffer, size );
		
	//free everything
	//CGContextRelease(context);	
	CGColorSpaceRelease( colorSpace );
    
	CGDataProviderRelease( prov );
	CGImageRelease( CGImage );
    
#ifdef PLATFORM_IOS
    if( alphaChannel ) //depremultiply the alpha dammit
    {
        byte* ptr = (byte*)bufptr;
        byte* end = ptr + width * height * 4;
        for( ; ptr < end; ptr += 4 )
        {
            float invAlpha = 1.f / (((float)ptr[3]) / 255.f);
            ptr[0] = (byte)(((float)ptr[0]) * invAlpha);
            ptr[1] = (byte)(((float)ptr[1]) * invAlpha);
            ptr[2] = (byte)(((float)ptr[2]) * invAlpha);
        }
    }
#endif
	return alphaChannel ? GL_RGBA : GL_RGB;
}

void ApplePlatform::_createApplicationDirectory()
{			
	//check if the directory exists, if not existing create it!
	NSFileManager *fileManager= [NSFileManager defaultManager]; 
	if(![ fileManager fileExistsAtPath:getAppDataPath().toNSString() ])
	{
		bool success = [fileManager 
						createDirectoryAtPath:getAppDataPath().toNSString()
						withIntermediateDirectories:YES 
						attributes:nil 
						error:NULL];
		
		DEBUG_ASSERT( success, "Cannot create the user data directory" );
	}	
}
