//
//  OSXPlatform.mm
//  dojo
//
//  Created by Tommaso Checchi on 5/7/11.
//  Copyright 2011 none. All rights reserved.
//

#include "OSXPlatform.h"

#import <ApplicationServices/ApplicationServices.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSOpenGL.h>
#import <AppKit/NSScreen.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSURL.h>

#include "Game.h"
#include "Utils.h"
#include "Table.h"
#include "FontSystem.h"
#include "SoundManager.h"

using namespace Dojo;

OSXPlatform::OSXPlatform( const Table& config ) :
ApplePlatform( config )
{
    screenWidth = [[NSScreen mainScreen] frame].size.width;
    screenHeight = [[NSScreen mainScreen] frame].size.height;
    screenOrientation = DO_LANDSCAPE_LEFT; //always
}


void OSXPlatform::initialise( Game* g )
{
    game = g;
	DEBUG_ASSERT( game );
	
	[NSApplication sharedApplication];
	
    pool = [[NSAutoreleasePool alloc] init];
	
    //store relevant paths
    mRootPath = [[NSBundle mainBundle] bundlePath];
    mResourcesPath = [[NSBundle mainBundle] resourcePath];
    
    NSArray* nspaths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
	mAppDataPath = [nspaths objectAtIndex:0];
    mAppDataPath += "/" + game->getName();
    
	_createApplicationDirectory();
    
    //override the config or load it from file
    if( config.size() == 0 )
        load( &config, getAppDataPath() + "/config.ds" );
    
    //override window size
    if( config.exists( "windowSize" ) )
    {
        Vector dim = config.getVector( "windowSize" );
        windowWidth = dim.x;
        windowHeight = dim.y;
    }
    else //use the defaults
    {
        windowWidth = game->getNativeWidth();
        windowHeight = game->getNativeHeight();
    }
    
    //auto-choose dimensions?
    if( windowWidth == 0 )  windowWidth = screenWidth;
    if( windowHeight == 0 ) windowHeight = screenHeight;
    
    //TODO read fullscreen
    
    NSRect frame;
    frame.origin.x = 10;
    frame.origin.y = 768;
    frame.size.width = windowWidth;
    frame.size.height = windowHeight;
	
    NSOpenGLPixelFormatAttribute attributes [] = {
        NSOpenGLPFAWindow,
        NSOpenGLPFADoubleBuffer,	// double buffered
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)16, // 16 bit depth buffer
		
		//msaa
		NSOpenGLPFASampleBuffers, 1,
		NSOpenGLPFASamples, (NSOpenGLPixelFormatAttribute)config.getInt( "MSAA" ),
		NSOpenGLPFANoRecovery,
		
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
		
    view = [[CustomOpenGLView alloc ]initWithFrame: frame pixelFormat: pixelformat ]; 
	
    [window setContentView: view];
	[window makeFirstResponder: view];
	
	[window makeKeyAndOrderFront:nil];
	
    //create render
    render = new Render( frame.size.width, frame.size.height, screenOrientation );
	
	//enable MSAA?
	if( config.getInt( "MSAA" ) )
		glEnable(GL_MULTISAMPLE);
    
    //create soundmanager
    sound = new SoundManager();
	
    //create input
    input = new InputSystem();
	
	//fonts
	fonts = new FontSystem();
	
	[view setPlatform:this];
        
    //launch the game
    game->begin();
}

OSXPlatform::~OSXPlatform()
{
	
}

void OSXPlatform::shutdown()
{    
	game->end();
	
	delete game;
	
    delete render;
    delete sound;
    delete input;
	delete fonts;
}

void OSXPlatform::prepareThreadContext()
{	
	NSOpenGLContext* context = [[NSOpenGLContext alloc]
								initWithFormat:[ view pixelFormat ]
								shareContext:[view openGLContext] ];
	
    [context makeCurrentContext];
}

void OSXPlatform::acquireContext()
{    
    [[view openGLContext] makeCurrentContext];
}
void OSXPlatform::present()
{    
    [[view openGLContext] flushBuffer];
}

void OSXPlatform::loop()
{		
	// start animation timer
	NSTimer* timer = [NSTimer 	timerWithTimeInterval:( game->getNativeFrameLength() )
								target:view 
								selector:@selector(stepCallback:) 
								userInfo:nil 
								repeats:YES];
	
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSEventTrackingRunLoopMode]; // ensure timer fires during resize
	
	//listen the window
	[window setDelegate:view];
	[window setAcceptsMouseMovedEvents:YES];
	
	running = true;
	
	//start event dispatching loop and give control to Cocoa
	[[NSApplication sharedApplication] run];
}

void OSXPlatform::setFullscreen(bool f)
{
    DEBUG_TODO;
}

void OSXPlatform::openWebPage( const String& site )
{
	NSURL* url = [NSURL URLWithString: site.toNSString() ];
	
	[[NSWorkspace sharedWorkspace] openURL: url ];
}
