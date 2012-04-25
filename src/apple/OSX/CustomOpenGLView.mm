//
//  StepCallback.m
//  dojo
//
//  Created by Tommaso Checchi on 5/15/11.
//  Copyright 2011 none. All rights reserved.
//

#import "CustomOpenGLView.h"

#import <AppKit/NSApplication.h>

#include "Platform.h"
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
	
	//set every repetition to 0
	memset( repetition, 0, sizeof( bool ) );
	
	//init keys
	// Virtual Key Map to KeyCode
	keymap[0x12] = InputSystem::KC_1;
	keymap[0x13] = InputSystem::KC_2;
	keymap[0x14] = InputSystem::KC_3;
	keymap[0x15] = InputSystem::KC_4;
	keymap[0x17] = InputSystem::KC_5;
	keymap[0x16] = InputSystem::KC_6;
	keymap[0x1A] = InputSystem::KC_7;
	keymap[0x1C] = InputSystem::KC_8;
	keymap[0x19] = InputSystem::KC_9;
	keymap[0x1D] = InputSystem::KC_0;
	
	keymap[0x33] = InputSystem::KC_BACK;  // might be wrong
	
	keymap[0x1B] = InputSystem::KC_MINUS;
	keymap[0x18] = InputSystem::KC_EQUALS;
	keymap[0x31] = InputSystem::KC_SPACE;
	keymap[0x2B] = InputSystem::KC_COMMA;
	keymap[0x2F] = InputSystem::KC_PERIOD;
	
	keymap[0x2A] = InputSystem::KC_BACKSLASH;
	keymap[0x2C] = InputSystem::KC_SLASH;
	keymap[0x21] = InputSystem::KC_LBRACKET;
	keymap[0x1E] = InputSystem::KC_RBRACKET;
	
	keymap[0x35] = InputSystem::KC_ESCAPE;
	keymap[0x39] = InputSystem::KC_CAPITAL;
	
	keymap[0x30] = InputSystem::KC_TAB;
	keymap[0x24] = InputSystem::KC_RETURN;  // double check return/enter
	
	//keymap[XK_colon] = InputSystem::KC_COLON;	 // no colon?
	keymap[0x29] = InputSystem::KC_SEMICOLON;
	keymap[0x27] = InputSystem::KC_APOSTROPHE;
	keymap[0x32] = InputSystem::KC_GRAVE;
	
	keymap[0x0B] = InputSystem::KC_B;
	keymap[0x00] = InputSystem::KC_A;
	keymap[0x08] = InputSystem::KC_C;
	keymap[0x02] = InputSystem::KC_D;
	keymap[0x0E] = InputSystem::KC_E;
	keymap[0x03] = InputSystem::KC_F;
	keymap[0x05] = InputSystem::KC_G;
	keymap[0x04] = InputSystem::KC_H;
	keymap[0x22] = InputSystem::KC_I;
	keymap[0x26] = InputSystem::KC_J;
	keymap[0x28] = InputSystem::KC_K;
	keymap[0x25] = InputSystem::KC_L;
	keymap[0x2E] = InputSystem::KC_M;
	keymap[0x2D] = InputSystem::KC_N;
	keymap[0x1F] = InputSystem::KC_O;
	keymap[0x23] = InputSystem::KC_P;
	keymap[0x0C] = InputSystem::KC_Q;
	keymap[0x0F] = InputSystem::KC_R;
	keymap[0x01] = InputSystem::KC_S;
	keymap[0x11] = InputSystem::KC_T;
	keymap[0x20] = InputSystem::KC_U;
	keymap[0x09] = InputSystem::KC_V;
	keymap[0x0D] = InputSystem::KC_W;
	keymap[0x07] = InputSystem::KC_X;
	keymap[0x10] = InputSystem::KC_Y;
	keymap[0x06] = InputSystem::KC_Z;
	
	keymap[0x7A] = InputSystem::KC_F1;
	keymap[0x78] = InputSystem::KC_F2;
	keymap[0x63] = InputSystem::KC_F3;
	keymap[0x76] = InputSystem::KC_F4;
	keymap[0x60] = InputSystem::KC_F5;
	keymap[0x61] = InputSystem::KC_F6;
	keymap[0x62] = InputSystem::KC_F7;
	keymap[0x64] = InputSystem::KC_F8;
	keymap[0x65] = InputSystem::KC_F9;
	keymap[0x6D] = InputSystem::KC_F10;
	keymap[0x67] = InputSystem::KC_F11;
	keymap[0x6F] = InputSystem::KC_F12;
	keymap[0x69] = InputSystem::KC_F13;
	keymap[0x6B] = InputSystem::KC_F14;
	keymap[0x71] = InputSystem::KC_F15;
	
	//Keypad
	keymap[0x52] = InputSystem::KC_NUMPAD0;
	keymap[0x53] = InputSystem::KC_NUMPAD1;
	keymap[0x54] = InputSystem::KC_NUMPAD2;
	keymap[0x55] = InputSystem::KC_NUMPAD3;
	keymap[0x56] = InputSystem::KC_NUMPAD4;
	keymap[0x57] = InputSystem::KC_NUMPAD5;
	keymap[0x58] = InputSystem::KC_NUMPAD6;
	keymap[0x59] = InputSystem::KC_NUMPAD7;
	keymap[0x5B] = InputSystem::KC_NUMPAD8;
	keymap[0x5C] = InputSystem::KC_NUMPAD9;
	keymap[0x45] = InputSystem::KC_ADD;
	keymap[0x4E] = InputSystem::KC_SUBTRACT;
	keymap[0x41] = InputSystem::KC_DECIMAL;
	keymap[0x51] = InputSystem::KC_NUMPADEQUALS;
	keymap[0x4B] = InputSystem::KC_DIVIDE;
	keymap[0x43] = InputSystem::KC_MULTIPLY;
	keymap[0x4C] = InputSystem::KC_NUMPADENTER;
	
	//Keypad with numlock off
	//keymap[0x73] = InputSystem::KC_NUMPAD7;  // not sure of these
	//keymap[XK_KP_Up] = InputSystem::KC_NUMPAD8; // check on a non-laptop
	//keymap[XK_KP_Page_Up] = InputSystem::KC_NUMPAD9;
	//keymap[XK_KP_Left] = InputSystem::KC_NUMPAD4;
	//keymap[XK_KP_Begin] = InputSystem::KC_NUMPAD5;
	//keymap[XK_KP_Right] = InputSystem::KC_NUMPAD6;
	//keymap[XK_KP_End] = InputSystem::KC_NUMPAD1;
	//keymap[XK_KP_Down] = InputSystem::KC_NUMPAD2;
	//keymap[XK_KP_Page_Down] = InputSystem::KC_NUMPAD3;
	//keymap[XK_KP_Insert] = InputSystem::KC_NUMPAD0;
	//keymap[XK_KP_Delete] = InputSystem::KC_DECIMAL;
	
	keymap[0x7E] = InputSystem::KC_UP;
	keymap[0x7D] = InputSystem::KC_DOWN;
	keymap[0x7B] = InputSystem::KC_LEFT;
	keymap[0x7C] = InputSystem::KC_RIGHT;
	
	keymap[0x74] = InputSystem::KC_PGUP;
	keymap[0x79] = InputSystem::KC_PGDOWN;
	keymap[0x73] = InputSystem::KC_HOME;
	keymap[0x77] = InputSystem::KC_END;
	
	//keymap[XK_Print] = InputSystem::KC_SYSRQ;		// ??
	//keymap[XK_Scroll_Lock] = InputSystem::KC_SCROLL; // ??
	//keymap[XK_Pause] = InputSystem::KC_PAUSE;		// ??
	
	
	//keymap[XK_Insert] = InputSystem::KC_INSERT;	  // ??
	keymap[0x75] = InputSystem::KC_DELETE; // del under help key?
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
	input->_fireMouseMoveEvent(Vector(	[theEvent locationInWindow].x, 
										  platform->getGame()->getNativeHeight() - [theEvent locationInWindow].y ) );
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

- (void)scrollWheel:(NSEvent *)theEvent
{
	input->_fireScrollWheelEvent( [theEvent deltaY] );
}


- (void)keyDown:(NSEvent *)theEvent
{	
	uint kc = [theEvent keyCode];
	if( !repetition[kc] ) {
		input->_fireKeyPressedEvent( [[theEvent characters] characterAtIndex:0], keymap[kc] );
		
		repetition[kc] = true;
	}		
}

- (void)keyUp:(NSEvent *)theEvent
{	
	input->_fireKeyReleasedEvent( [[theEvent characters] characterAtIndex:0], keymap[[theEvent keyCode] ] );
	
	repetition[[theEvent keyCode]] = false;
}

- (void)dealloc
{
    [super dealloc];
}

@end
