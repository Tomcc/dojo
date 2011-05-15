//
//  OSXPlatform.mm
//  dojo
//
//  Created by Tommaso Checchi on 5/7/11.
//  Copyright 2011 none. All rights reserved.
//

#include "OSXPlatform.h"

#include <OIS/OIS.h>

#import <ApplicationServices/ApplicationServices.h>
#import <AppKit/NSImage.h>
#import <Foundation/NSTimer.h>

#include "Game.h"
#include "Utils.h"


using namespace Dojo;

void OSXPlatform::initialise()
{	
	DEBUG_ASSERT( game );
	
	[NSApplication sharedApplication];
	
    pool = [[NSAutoreleasePool alloc] init];
	
	//show menu bar
	NSMenu* menu = [[NSMenu alloc] initWithTitle: Utils::toNSString( game->getName() ) ];
	 [[NSApplication sharedApplication] setMenu:menu];
	
    NSRect frame;
    frame.origin.x = 10;
    frame.origin.y = 10;
    frame.size.width = game->getNativeWidth();
    frame.size.height = game->getNativeHeight();
	
    NSOpenGLPixelFormatAttribute attributes [] = {
        NSOpenGLPFAWindow,
        NSOpenGLPFADoubleBuffer,	// double buffered
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)16, // 16 bit depth buffer
        (NSOpenGLPixelFormatAttribute)nil
    };
	
    //create the window
	window = [[NSWindow alloc]
              initWithContentRect: frame
              styleMask: (NSTitledWindowMask | NSMiniaturizableWindowMask | NSClosableWindowMask)
              backing: NSBackingStoreBuffered
              defer: YES];
	
	[window setReleasedWhenClosed:false];
	    
	NSOpenGLPixelFormat* pixelformat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
	
    view = [[NSOpenGLView alloc ]initWithFrame: frame pixelFormat: pixelformat ]; 
	
    [window setContentView: view];
	
	[window makeKeyAndOrderFront:nil];
	
    //create render
    render = new Render( frame.size.width, frame.size.height, 1, Render::RO_LANDSCAPE_LEFT );
    
    //create soundmanager
    sound = new SoundManager();
    
    //create input
    input = new TouchSource();
    
    //launch the game
    game->onBegin();
}

OSXPlatform::~OSXPlatform()
{
	[pool release];
}

void OSXPlatform::shutdown()
{
	[callback release];
    
	game->onEnd();
	
	delete game;
	
    delete render;
    delete sound;
    delete input;
}

void OSXPlatform::acquireContext()
{    
    [[view openGLContext] makeCurrentContext];
}
void OSXPlatform::present()
{    
    [[view openGLContext] flushBuffer];
}

void OSXPlatform::step( float dt )
{    
    game->onLoop(dt);
    
    render->render();
    sound->update(dt);
}

void OSXPlatform::loop( float frameTime )
{	
	//listen the window
	callback = [[GenericListener alloc] initWithPlatform:this];
		
	// start animation timer
	NSTimer* timer = [NSTimer 	timerWithTimeInterval:( frameTime ) 
								target:callback 
								selector:@selector(stepCallback:) 
								userInfo:nil 
								repeats:YES];
	
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSEventTrackingRunLoopMode]; // ensure timer fires during resize
	
	//listen the window
	[window setDelegate:callback];
	
	//start event dispatching loop and give control to Cocoa
	[[NSApplication sharedApplication] run];
}

std::string OSXPlatform::getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path )
{
	NSString* NSName = Utils::toNSString( name );
	NSString* NSType = Utils::toNSString( type );
	NSString* NSPath = Utils::toNSString( path );
	
	NSString* res = [[NSBundle mainBundle] pathForResource:NSName ofType:NSType inDirectory:NSPath ];
	
	[NSName release];
	[NSType release];
	[NSPath release];
	
	if( res )
		return Utils::toSTDString( res );
	else
		return "";
}


void OSXPlatform::getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out )
{	
	DEBUG_ASSERT( type.size() );
	DEBUG_ASSERT( path.size() );
	
    NSString* nstype = Utils::toNSString( type );
	NSString* nspath = Utils::toNSString( path );
	
	NSArray* paths = [[NSBundle mainBundle] pathsForResourcesOfType:nstype inDirectory:nspath];
	
	//convert array
	for( int i = 0; i < [paths count]; ++i )
		out.push_back( Utils::toSTDString( [paths objectAtIndex:i] ) );
	
	[paths release];
	[nspath release];
	[nstype release];
}


uint OSXPlatform::OSXPlatform::loadFileContent( char*& bufptr, const std::string& path )
{
    bufptr = NULL;
	
	DEBUG_ASSERT( path.size() );
	
	NSString* NSPath = Utils::toNSString( path );
	
	NSData* data = [[NSData alloc] initWithContentsOfFile: NSPath ];
	
	if( !data )
		return false;
	
	uint size = (uint)[data length];
	
	//alloc the new buffer
	bufptr = (char*)malloc( size );
	memcpy( bufptr, [data bytes], size );
	
	[data release];
	
	return size;
}


void OSXPlatform::loadPNGContent( void*& imageData, const std::string& path, uint& width, uint& height )
{
	width = height = 0;
	
	NSString* imgPath = Utils::toNSString( path );
	//magic OBJC code
	NSData *texData = [[NSData alloc] initWithContentsOfFile: imgPath ];
	
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
	
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	imageData = malloc( internalWidth * internalHeight * 4 );
	
	memset( imageData, 0, internalWidth * internalHeight * 4 );
	
	CGContextRef context = CGBitmapContextCreate(imageData, 
												 internalWidth, 
												 internalHeight, 
												 8, 
												 4 * internalWidth, 
												 colorSpace, 
												 kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big );
	
	CGContextClearRect( context, CGRectMake( 0, 0, internalWidth, internalHeight ) );
	CGContextTranslateCTM( context, 0, internalHeight - height );
	CGContextDrawImage( context, CGRectMake( 0, 0, width, height ), CGImage );
	
	//free everything
	CGContextRelease(context);	
	CGColorSpaceRelease( colorSpace );
	CGImageRelease( CGImage );
	CGDataProviderRelease( prov );
	[texData release];
}



///loads the given file in a buffer - WARNING not every format is supported on every platform
uint OSXPlatform::loadAudioFileContent( ALuint& buffer, const std::string& path )
{
    return DEBUG_TODO;
}



void OSXPlatform::load( Table* dest, const std::string& relPath )
{
    DEBUG_TODO;
}

void OSXPlatform::save( Table* table, const std::string& relPath )
{
    DEBUG_TODO;
}

void OSXPlatform::openWebPage( const std::string& site )
{
    DEBUG_TODO;
}