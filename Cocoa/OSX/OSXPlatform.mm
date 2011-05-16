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
#import <Foundation/NSURL.h>

#include "Game.h"
#include "Utils.h"
#include "Table.h"

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

///loads the given file in a buffer - WARNING not every format is supported on every platform
uint OSXPlatform::loadAudioFileContent( ALuint& buffer, const std::string& path )
{
    return DEBUG_TODO;
}