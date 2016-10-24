//
//  ApplePlatform.cpp
//  dojo
//
//  Created by Tommaso Checchi on 5/16/11.
//  Copyright 2011 none. All rights reserved.
//

#include "ApplePlatform.h"

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/NSLocale.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSFileManager.h>

#ifdef PLATFORM_OSX
	#import <ApplicationServices/ApplicationServices.h>
#else
	#import <CoreGraphics/CoreGraphics.h>
#endif

#include "Timer.h"
#include "Renderer.h"
#include "SoundManager.h"
#include "InputSystem.h"
#include "Game.h"
#include "Table.h"
#include "dojostring.h"
#include "StringReader.h"
#include "Path.h"

using namespace Dojo;
using namespace std;

ApplePlatform::ApplePlatform( const Table& config ) :
Platform( config )
{
    pool = [[NSAutoreleasePool alloc] init];

	if( [[NSLocale preferredLanguages] count ] )
        locale += [[[NSLocale preferredLanguages ] objectAtIndex:0] UTF8String];
	else
		locale += "en";
}

ApplePlatform::~ApplePlatform()
{
	[pool release];
}

void ApplePlatform::step( float dt )
{
    frameTimer.reset();

	//clamp to max dt to avoid crazy behaviour
	dt = Math::clamp( dt, game->getNativeFrameLength(), game->getMaximumFrameLength() );

//    mBackgroundQueue->fireCompletedCallbacks();

    input->poll(dt);

    game->loop(dt);

    sound->update(dt);

    render->renderFrame(dt);
    
    float elapsedTime = frameTimer.getElapsedTime();
    
    _runASyncTasks(elapsedTime);
    realFrameTime = elapsedTime;
    
    render->endFrame();
}

PixelFormat ApplePlatform::loadImageFile( std::vector<uint8_t>& imageData, utf::string_view path, uint32_t& width, uint32_t& height, int& pixelSize)
{
    width = height = 0;
    
    NSString *nsPath = [NSString stringWithUTF8String:path.data()];
	NSURL* url = [NSURL fileURLWithPath: nsPath ];

	CGDataProviderRef prov = CGDataProviderCreateWithURL( (CFURLRef)url );

	if( prov == nil )
        return PixelFormat::Unknown;

	CGImageRef CGImage;

    utf::string_view ext = Path::getFileExtension( path );

    if( ext == "png" )
        CGImage = CGImageCreateWithPNGDataProvider( prov, NULL, true, kCGRenderingIntentDefault );
    else if( ext == "jpg" )
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
    NSString* utfDataPath = [NSString stringWithUTF8String:getAppDataPath().data()];
	if(![ fileManager fileExistsAtPath: utfDataPath ])
	{
		bool success = [fileManager
						createDirectoryAtPath:utfDataPath
						withIntermediateDirectories:YES
						attributes:nil
						error:NULL];

		DEBUG_ASSERT( success, "Cannot create the user data directory" );
	}
}
