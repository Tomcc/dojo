//
//  StepCallback.m
//  dojo
//
//  Created by Tommaso Checchi on 5/15/11.
//  Copyright 2011 none. All rights reserved.
//

#import "CustomOpenGLView.h"

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
	
	//init keys
	// Virtual Key Map to KeyCode
	keymap[0x12] = TouchSource::KC_1;
	keymap[0x13] = TouchSource::KC_2;
	keymap[0x14] = TouchSource::KC_3;
	keymap[0x15] = TouchSource::KC_4;
	keymap[0x17] = TouchSource::KC_5;
	keymap[0x16] = TouchSource::KC_6;
	keymap[0x1A] = TouchSource::KC_7;
	keymap[0x1C] = TouchSource::KC_8;
	keymap[0x19] = TouchSource::KC_9;
	keymap[0x1D] = TouchSource::KC_0;
	
	keymap[0x33] = TouchSource::KC_BACK;  // might be wrong
	
	keymap[0x1B] = TouchSource::KC_MINUS;
	keymap[0x18] = TouchSource::KC_EQUALS;
	keymap[0x31] = TouchSource::KC_SPACE;
	keymap[0x2B] = TouchSource::KC_COMMA;
	keymap[0x2F] = TouchSource::KC_PERIOD;
	
	keymap[0x2A] = TouchSource::KC_BACKSLASH;
	keymap[0x2C] = TouchSource::KC_SLASH;
	keymap[0x21] = TouchSource::KC_LBRACKET;
	keymap[0x1E] = TouchSource::KC_RBRACKET;
	
	keymap[0x35] = TouchSource::KC_ESCAPE;
	keymap[0x39] = TouchSource::KC_CAPITAL;
	
	keymap[0x30] = TouchSource::KC_TAB;
	keymap[0x24] = TouchSource::KC_RETURN;  // double check return/enter
	
	//keymap[XK_colon] = TouchSource::KC_COLON;	 // no colon?
	keymap[0x29] = TouchSource::KC_SEMICOLON;
	keymap[0x27] = TouchSource::KC_APOSTROPHE;
	keymap[0x32] = TouchSource::KC_GRAVE;
	
	keymap[0x0B] = TouchSource::KC_B;
	keymap[0x00] = TouchSource::KC_A;
	keymap[0x08] = TouchSource::KC_C;
	keymap[0x02] = TouchSource::KC_D;
	keymap[0x0E] = TouchSource::KC_E;
	keymap[0x03] = TouchSource::KC_F;
	keymap[0x05] = TouchSource::KC_G;
	keymap[0x04] = TouchSource::KC_H;
	keymap[0x22] = TouchSource::KC_I;
	keymap[0x26] = TouchSource::KC_J;
	keymap[0x28] = TouchSource::KC_K;
	keymap[0x25] = TouchSource::KC_L;
	keymap[0x2E] = TouchSource::KC_M;
	keymap[0x2D] = TouchSource::KC_N;
	keymap[0x1F] = TouchSource::KC_O;
	keymap[0x23] = TouchSource::KC_P;
	keymap[0x0C] = TouchSource::KC_Q;
	keymap[0x0F] = TouchSource::KC_R;
	keymap[0x01] = TouchSource::KC_S;
	keymap[0x11] = TouchSource::KC_T;
	keymap[0x20] = TouchSource::KC_U;
	keymap[0x09] = TouchSource::KC_V;
	keymap[0x0D] = TouchSource::KC_W;
	keymap[0x07] = TouchSource::KC_X;
	keymap[0x10] = TouchSource::KC_Y;
	keymap[0x06] = TouchSource::KC_Z;
	
	keymap[0x7A] = TouchSource::KC_F1;
	keymap[0x78] = TouchSource::KC_F2;
	keymap[0x63] = TouchSource::KC_F3;
	keymap[0x76] = TouchSource::KC_F4;
	keymap[0x60] = TouchSource::KC_F5;
	keymap[0x61] = TouchSource::KC_F6;
	keymap[0x62] = TouchSource::KC_F7;
	keymap[0x64] = TouchSource::KC_F8;
	keymap[0x65] = TouchSource::KC_F9;
	keymap[0x6D] = TouchSource::KC_F10;
	keymap[0x67] = TouchSource::KC_F11;
	keymap[0x6F] = TouchSource::KC_F12;
	keymap[0x69] = TouchSource::KC_F13;
	keymap[0x6B] = TouchSource::KC_F14;
	keymap[0x71] = TouchSource::KC_F15;
	
	//Keypad
	keymap[0x52] = TouchSource::KC_NUMPAD0;
	keymap[0x53] = TouchSource::KC_NUMPAD1;
	keymap[0x54] = TouchSource::KC_NUMPAD2;
	keymap[0x55] = TouchSource::KC_NUMPAD3;
	keymap[0x56] = TouchSource::KC_NUMPAD4;
	keymap[0x57] = TouchSource::KC_NUMPAD5;
	keymap[0x58] = TouchSource::KC_NUMPAD6;
	keymap[0x59] = TouchSource::KC_NUMPAD7;
	keymap[0x5B] = TouchSource::KC_NUMPAD8;
	keymap[0x5C] = TouchSource::KC_NUMPAD9;
	keymap[0x45] = TouchSource::KC_ADD;
	keymap[0x4E] = TouchSource::KC_SUBTRACT;
	keymap[0x41] = TouchSource::KC_DECIMAL;
	keymap[0x51] = TouchSource::KC_NUMPADEQUALS;
	keymap[0x4B] = TouchSource::KC_DIVIDE;
	keymap[0x43] = TouchSource::KC_MULTIPLY;
	keymap[0x4C] = TouchSource::KC_NUMPADENTER;
	
	//Keypad with numlock off
	//keymap[0x73] = TouchSource::KC_NUMPAD7;  // not sure of these
	//keymap[XK_KP_Up] = TouchSource::KC_NUMPAD8; // check on a non-laptop
	//keymap[XK_KP_Page_Up] = TouchSource::KC_NUMPAD9;
	//keymap[XK_KP_Left] = TouchSource::KC_NUMPAD4;
	//keymap[XK_KP_Begin] = TouchSource::KC_NUMPAD5;
	//keymap[XK_KP_Right] = TouchSource::KC_NUMPAD6;
	//keymap[XK_KP_End] = TouchSource::KC_NUMPAD1;
	//keymap[XK_KP_Down] = TouchSource::KC_NUMPAD2;
	//keymap[XK_KP_Page_Down] = TouchSource::KC_NUMPAD3;
	//keymap[XK_KP_Insert] = TouchSource::KC_NUMPAD0;
	//keymap[XK_KP_Delete] = TouchSource::KC_DECIMAL;
	
	keymap[0x7E] = TouchSource::KC_UP;
	keymap[0x7D] = TouchSource::KC_DOWN;
	keymap[0x7B] = TouchSource::KC_LEFT;
	keymap[0x7C] = TouchSource::KC_RIGHT;
	
	keymap[0x74] = TouchSource::KC_PGUP;
	keymap[0x79] = TouchSource::KC_PGDOWN;
	keymap[0x73] = TouchSource::KC_HOME;
	keymap[0x77] = TouchSource::KC_END;
	
	//keymap[XK_Print] = TouchSource::KC_SYSRQ;		// ??
	//keymap[XK_Scroll_Lock] = TouchSource::KC_SCROLL; // ??
	//keymap[XK_Pause] = TouchSource::KC_PAUSE;		// ??
	
	
	//keymap[XK_Insert] = TouchSource::KC_INSERT;	  // ??
	keymap[0x75] = TouchSource::KC_DELETE; // del under help key?
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
	
	input->_fireKeyPressedEvent( [[theEvent characters] characterAtIndex:0], keymap[[theEvent keyCode]] );
}

- (void)keyUp:(NSEvent *)theEvent
{	
	NSLog([theEvent characters]);
	
	input->_fireKeyReleasedEvent( [[theEvent characters] characterAtIndex:0], keymap[[theEvent keyCode] ] );	
}

- (void)dealloc
{
    [super dealloc];
}

@end
