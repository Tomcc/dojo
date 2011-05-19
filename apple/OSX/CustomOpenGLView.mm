//
//  StepCallback.m
//  dojo
//
//  Created by Tommaso Checchi on 5/15/11.
//  Copyright 2011 none. All rights reserved.
//

#import "CustomOpenGLView.h"

#include "Platform.h"
#include "TouchSource.h"
#include "Vector.h"
#include "Game.h"

using namespace Dojo;

@implementation CustomOpenGLView

- (void)setPlatform:(Dojo::Platform*)targetPlatform
{    
	DEBUG_ASSERT(targetPlatform);
	DEBUG_ASSERT(targetPlatform->getInput() );		
		
	platform = targetPlatform;
	input = platform->getInput();
		
	time = CFAbsoluteTimeGetCurrent();
}

- (void)windowWillClose:(NSNotification *)notification
{
	//shutdown the app and let the Platform continue
	[[NSApplication sharedApplication] stop:self];
}

- (void)stepCallback:(NSTimer *)timer
{
	CFTimeInterval deltaTime = CFAbsoluteTimeGetCurrent() - time;
	
	time = CFAbsoluteTimeGetCurrent();
	
	//HACK - use real elapsed time
	platform->step( deltaTime );
}

- (void)mouseDown: (NSEvent *)theEvent
{
	input->_fireTouchBeginEvent( Vector( 
										[theEvent locationInWindow].x, 
										platform->getGame()->getNativeHeight() - [theEvent locationInWindow].y ) );
}

- (void)mouseDragged: (NSEvent *)theEvent
{
	input->_fireTouchMoveEvent( Vector( 
									   [theEvent locationInWindow].x, 
									   platform->getGame()->getNativeHeight() - [theEvent locationInWindow].y ) );
}

- (void)mouseUp: (NSEvent *)theEvent
{
	input->_fireTouchEndEvent( Vector( 
									  [theEvent locationInWindow].x, 
									  platform->getGame()->getNativeHeight() - [theEvent locationInWindow].y ) );	
}

- (void)mouseMoved: (NSEvent *)theEvent
{
	
}

- (void)rightMouseDown: (NSEvent *)theEvent
{
	
}

- (void)rightMouseDragged: (NSEvent *)theEvent
{
	
}

- (void)rightMouseUp: (NSEvent *)theEvent
{
	
}

- (void)otherMouseDown: (NSEvent *)theEvent
{
	
}

- (void)otherMouseDragged: (NSEvent *)theEvent
{
	
}

- (void)otherMouseUp: (NSEvent *)theEvent
{
	
}


- (void)keyDown:(NSEvent *)theEvent
{
	NSLog([theEvent characters]);
	
	input->_fireKeyPressedEvent( [[theEvent characters] characterAtIndex:0], [theEvent keyCode] );
}

- (void)keyUp:(NSEvent *)theEvent
{	
	NSLog([theEvent characters]);
	
	input->_fireKeyReleasedEvent( [[theEvent characters] characterAtIndex:0], [theEvent keyCode] );	
}

- (void)dealloc
{
    [super dealloc];
}

@end
