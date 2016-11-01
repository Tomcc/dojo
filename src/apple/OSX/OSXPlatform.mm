//
//  OSXPlatform.mm
//  dojo
//
//  Created by Tommaso Checchi on 5/7/11.
//  Copyright 2011 none. All rights reserved.
//

#include "apple/OSX/OSXPlatform.h"

#import <ApplicationServices/ApplicationServices.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSOpenGL.h>
#import <AppKit/NSScreen.h>
#import <AppKit/NSMenuItem.h>
#import <AppKit/NSMenu.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSURL.h>

#include "Game.h"
// BTT needs to be removed?
//#include "Utils.h"
#include "Table.h"
#include "FontSystem.h"
#include "SoundManager.h"
#include "RenderSurface.h"
#include "glad/glad.h"
// BTT needs to be removed?
//#include "BackgroundQueue.h"

#undef self

#ifndef GL_MULTISAMPLE
// Avoid inclusion of Windows Header
    #define GL_MULTISAMPLE  0x809D
#endif

using namespace Dojo;

OSXPlatform::OSXPlatform( const Table& config ) :
    ApplePlatform( config ),
    dragging(false),
    mMousePressed(false),
    cursorPos(Vector::Zero),
    frameInterval(0),
    mFramesToAdvance(0),
    clientAreaYOffset(0)
{
    /* ApplePlatform inits the NSAutoreleasePool and sets the locale to "en" */

    screenWidth = [[NSScreen mainScreen] frame].size.width;
    screenHeight = [[NSScreen mainScreen] frame].size.height;
    screenOrientation = DO_LANDSCAPE_LEFT; //always

    _initKeyMap();
}


void OSXPlatform::initialize( Unique<Game> g )
{
    DEBUG_ASSERT( g, "The game implementation cannot be null in initialize()" );

    game = move(g);

	[NSApplication sharedApplication];
	
    pool = [[NSAutoreleasePool alloc] init];
	
    //store relevant paths
    mRootPath = [[[NSBundle mainBundle] bundlePath] UTF8String];
    mResourcesPath = [[[NSBundle mainBundle] resourcePath] UTF8String];
    
    NSArray* nspaths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    mAppDataPath = [[NSString stringWithFormat:@"%@%s%s", [nspaths objectAtIndex:0], "/", game->getName().data()] UTF8String];
    
	_createApplicationDirectory();
    
    //create the standard app menu
    {
        id menubar = [[NSMenu new] autorelease];
        id appMenuItem = [[NSMenuItem new] autorelease];
        [menubar addItem:appMenuItem];
        [NSApp setMainMenu:menubar];
        id appMenu = [[NSMenu new] autorelease];
        id appName = [[NSProcessInfo processInfo] processName];
        id quitTitle = [@"Quit " stringByAppendingString:appName];
        id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
        [appMenu addItem:quitMenuItem];
        [appMenuItem setSubmenu:appMenu];
    }
    
    //override the config or load it from file
    Table userConfig = Table::loadFromFile(getAppDataPath() + "/config.ds");

    config.inherit( &userConfig ); //merge the table loaded from file and override with hardcoded directives
    
    //get the right screen
    NSScreen* screen = [NSScreen mainScreen];
    if( config.exists("screen" ) )
    {
        int screenID = config.getInt("screen");
        
        if( screenID < [[NSScreen screens] count] )
        {
            screen = [[NSScreen screens] objectAtIndex:screenID ];
            screenWidth = [screen frame].size.width;
            screenHeight = [screen frame].size.height;
            
            game->onWindowResolutionChanged( screenWidth, screenHeight );
        }
    }
        
    //override window size
    Vector dim = config.getVector( "windowSize", Vector( game->getNativeWidth(), game->getNativeHeight() ) );
    windowWidth = dim.x;
    windowHeight = dim.y;
    
    //auto-choose dimensions?
    if( windowWidth == 0 )  windowWidth = screenWidth;
    if( windowHeight == 0 ) windowHeight = screenHeight;

    mFullscreen = (windowWidth == screenWidth) and (windowHeight == screenHeight) and config.getBool("fullscreen");

    NSRect frame;
    frame.origin.x = 0;
    frame.origin.y = 0;
    frame.size.width = windowWidth;
    frame.size.height = windowHeight;
	
    NSOpenGLPixelFormatAttribute attributes [] = {
//        NSOpenGLPFAWindow, TODO Remove -- deprecated from 10.9
        NSOpenGLPFADoubleBuffer,	// double buffered
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)16, // 16 bit depth buffer
		
		//msaa
		NSOpenGLPFASampleBuffers, 1,
		NSOpenGLPFASamples, (NSOpenGLPixelFormatAttribute)config.getInt( "MSAA" ),
		NSOpenGLPFANoRecovery,
		
        /*(NSOpenGLPixelFormatAttribute)*/0
    };

    NSUInteger style = config.getBool("noTitleBar") ? (NSWindowStyleMaskBorderless) : (NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskClosable);

    //create the window
	window = [[NSWindow alloc]
              initWithContentRect: frame
              styleMask: style
              backing: NSBackingStoreBuffered
              defer: YES
              screen:screen];
	
	[window setReleasedWhenClosed:false];
	    
	NSOpenGLPixelFormat* pixelformat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];

    view = [[CustomOpenGLView alloc ]initWithFrame: frame pixelFormat: pixelformat ];

    [window setContentView: view];
	[window makeFirstResponder: view];
	
	[window makeKeyAndOrderFront:nil];
	
    //create render
    render = make_unique<Renderer>(
        RenderSurface
        {
            windowWidth,
            windowHeight,
            PixelFormat::RGBA_8_8_8_8
        },
        DO_LANDSCAPE_LEFT
    );

	//enable MSAA?
	if( config.getInt( "MSAA" ) )
		glEnable(GL_MULTISAMPLE);

//    int userThreadOverride = config.getNumber( "threads", -1 );
//    mBackgroundQueue = new BackgroundQueue( userThreadOverride );

    //create soundmanager
    sound = make_unique<SoundManager>();

    //create input and the keyboard system object
    input = make_unique<InputSystem>();

	//fonts
	fonts = make_unique<FontSystem>();

	[view setPlatform:this];

    DEBUG_MESSAGE("---- Launching game!");

    //launch the game
    game->begin();
}

OSXPlatform::~OSXPlatform()
{
	
}

void OSXPlatform::shutdown()
{    
	game->end();

//	delete game; TODO Remove since it's now moved
/* No more delete with smart ptrs
    delete render;
    delete sound;
    delete input;
	delete fonts;
 */
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
	realFrameTime = frameTimer.getElapsedTime();
    
    [[view openGLContext] flushBuffer];
}

void OSXPlatform::loop()
{
    frameTimer.reset();
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

void OSXPlatform::step(float dt)
{
    mStepTimer.reset();
    
    // update input
    _pollDevices(dt);
    
    game->loop(dt);
    
    sound->update(dt);
    
    render->renderFrame(dt);
    
    float elapsed = (float)mStepTimer.getElapsedTime();
    _runASyncTasks(elapsed);
    //take the time before swapBuffers because on some implementations it is blocking
    realFrameTime = elapsed;
    
    render->endFrame(); //present the frame
}

void OSXPlatform::submitFrame()
{
    [[view openGLContext] flushBuffer];
}

PixelFormat OSXPlatform::loadImageFile(std::vector<uint8_t> &imageData, utf::string_view path, uint32_t &width, uint32_t &height, int &pixelSize)
{
    return ApplePlatform::loadImageFile(imageData, path, width, height, pixelSize);
}

void OSXPlatform::setFullscreen(bool f)
{

    if(f == mFullscreen)
    {
        return;
    }

    _setFullScreen(f);

    mFullscreen = f;

    //store the new settings into config.ds
    config.set("fullscreen", mFullscreen);
    save(config, "config");
}

void OSXPlatform::_setFullScreen(bool fullscreen)
{
    NSWindowCollectionBehavior behavior = [window collectionBehavior];

    if( not fullscreen){
        behavior ^= NSWindowCollectionBehaviorFullScreenPrimary;
    }else{
        behavior |= NSWindowCollectionBehaviorFullScreenPrimary;
    }
    
    [window setCollectionBehavior:behavior];

}

void OSXPlatform::_pollDevices(float dt)
{
    input->poll(dt);
    
    DEBUG_TODO
}

void OSXPlatform::setMouseLocked(bool locked)
{
    if(mouseLocked != locked)
    {
//        ShowCursor(not locked);
//        ShowCursor(not locked);
        
        mouseLocked = locked;
    }
}

utf::string_view OSXPlatform::getPicturesPath() {
    DEBUG_TODO
    return mPicturesPath;
}

utf::string_view OSXPlatform::getShaderCachePath()
{
    return mShaderCachePath;
}

void OSXPlatform::openWebPage( const utf::string_view site )
{
	NSURL* url = [NSURL URLWithString: [NSString stringWithUTF8String:site.data()]];
	
	[[NSWorkspace sharedWorkspace] openURL: url ];
}

bool OSXPlatform::isNPOTEnabled()
{
    return true;
}

//init key map
void OSXPlatform::_initKeyMap() {
}
