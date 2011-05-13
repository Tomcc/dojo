//
//  OSXPlatform.mm
//  dojo
//
//  Created by Tommaso Checchi on 5/7/11.
//  Copyright 2011 none. All rights reserved.
//

#include "OSXPlatform.h"

#include <OIS/OIS.h>

#include "Game.h"

using namespace Dojo;


void OSXPlatform::initialise()
{
    DEBUG_ASSERT( game );
    
    //create the application
    [NSApplication sharedApplication];
    
    
    NSRect frame;
    frame.origin.x = 100;
    frame.origin.y = 100;
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
    
    NSOpenGLPixelFormat* pixelformat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
    
    view = [[NSOpenGLView alloc ]initWithFrame: frame pixelFormat: pixelformat ];    
    
    [window setContentView: view];
    
    [window makeKeyAndOrderFront:nil];
    
    //do magic and run the dispatcher on the main thread
    [NSApp
     performSelectorOnMainThread:@selector(run)
     withObject:nil
     waitUntilDone:NO];
}

void OSXPlatform::shutdown()
{
    DEBUG_TODO;    
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
    DEBUG_TODO;
}


void OSXPlatform::loop( float frameTime )
{
    while(1)
    {
        
    }
}

std::string OSXPlatform::getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path )
{
    DEBUG_TODO;
    
    return "";
}


void OSXPlatform::getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out )
{
    DEBUG_TODO;
}


uint OSXPlatform::OSXPlatform::loadFileContent( char*& bufptr, const std::string& path )
{
    return DEBUG_TODO;
}


void OSXPlatform::loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height )
{
    DEBUG_TODO;
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