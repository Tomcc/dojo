//
//  StepCallback.m
//  dojo
//
//  Created by Tommaso Checchi on 5/15/11.
//  Copyright 2011 none. All rights reserved.
//

#import "apple/OSX/CustomOpenGLView.h"

#import <AppKit/NSApplication.h>

#include "Platform.h"
#include "Vector.h"
#include "Game.h"
#include "Keyboard.h"
#include "Touch.h"
#undef self

using namespace Dojo;

@implementation CustomOpenGLView

- (void)setPlatform:(Dojo::Platform*)targetPlatform
{    
	DEBUG_ASSERT(targetPlatform, "The platform can't be null" );
	DEBUG_ASSERT(&targetPlatform->getInput(), "The input system can't be null" );
		
	platform = targetPlatform;
	input = &platform->getInput();
    keyboard = make_unique<Keyboard>();
    input->addDevice( *keyboard );
		
	time = CFAbsoluteTimeGetCurrent();
	
	//init keys
	// Virtual Key Map to KeyCode
	keymap[0x12] = KC_1;
	keymap[0x13] = KC_2;
	keymap[0x14] = KC_3;
	keymap[0x15] = KC_4;
	keymap[0x17] = KC_5;
	keymap[0x16] = KC_6;
	keymap[0x1A] = KC_7;
	keymap[0x1C] = KC_8;
	keymap[0x19] = KC_9;
	keymap[0x1D] = KC_0;
	
	keymap[0x33] = KC_BACK;  // might be wrong
	
	keymap[0x1B] = KC_MINUS;
	keymap[0x18] = KC_EQUALS;
	keymap[0x31] = KC_SPACE;
	keymap[0x2B] = KC_COMMA;
	keymap[0x2F] = KC_PERIOD;
	
	keymap[0x2A] = KC_BACKSLASH;
	keymap[0x2C] = KC_SLASH;
	keymap[0x21] = KC_LBRACKET;
	keymap[0x1E] = KC_RBRACKET;
	
	keymap[0x35] = KC_ESCAPE;
	keymap[0x39] = KC_CAPITAL;
	
	keymap[0x30] = KC_TAB;
	keymap[0x24] = KC_RETURN;  // double check return/enter
	
	//keymap[XK_colon] = KC_COLON;	 // no colon?
	keymap[0x29] = KC_SEMICOLON;
	keymap[0x27] = KC_APOSTROPHE;
	keymap[0x32] = KC_GRAVE;
	
	keymap[0x0B] = KC_B;
	keymap[0x00] = KC_A;
	keymap[0x08] = KC_C;
	keymap[0x02] = KC_D;
	keymap[0x0E] = KC_E;
	keymap[0x03] = KC_F;
	keymap[0x05] = KC_G;
	keymap[0x04] = KC_H;
	keymap[0x22] = KC_I;
	keymap[0x26] = KC_J;
	keymap[0x28] = KC_K;
	keymap[0x25] = KC_L;
	keymap[0x2E] = KC_M;
	keymap[0x2D] = KC_N;
	keymap[0x1F] = KC_O;
	keymap[0x23] = KC_P;
	keymap[0x0C] = KC_Q;
	keymap[0x0F] = KC_R;
	keymap[0x01] = KC_S;
	keymap[0x11] = KC_T;
	keymap[0x20] = KC_U;
	keymap[0x09] = KC_V;
	keymap[0x0D] = KC_W;
	keymap[0x07] = KC_X;
	keymap[0x10] = KC_Y;
	keymap[0x06] = KC_Z;
	
	keymap[0x7A] = KC_F1;
	keymap[0x78] = KC_F2;
	keymap[0x63] = KC_F3;
	keymap[0x76] = KC_F4;
	keymap[0x60] = KC_F5;
	keymap[0x61] = KC_F6;
	keymap[0x62] = KC_F7;
	keymap[0x64] = KC_F8;
	keymap[0x65] = KC_F9;
	keymap[0x6D] = KC_F10;
	keymap[0x67] = KC_F11;
	keymap[0x6F] = KC_F12;
	keymap[0x69] = KC_F13;
	keymap[0x6B] = KC_F14;
	keymap[0x71] = KC_F15;
	
	//Keypad
	keymap[0x52] = KC_NUMPAD0;
	keymap[0x53] = KC_NUMPAD1;
	keymap[0x54] = KC_NUMPAD2;
	keymap[0x55] = KC_NUMPAD3;
	keymap[0x56] = KC_NUMPAD4;
	keymap[0x57] = KC_NUMPAD5;
	keymap[0x58] = KC_NUMPAD6;
	keymap[0x59] = KC_NUMPAD7;
	keymap[0x5B] = KC_NUMPAD8;
	keymap[0x5C] = KC_NUMPAD9;
	keymap[0x45] = KC_ADD;
	keymap[0x4E] = KC_SUBTRACT;
	keymap[0x41] = KC_DECIMAL;
	keymap[0x51] = KC_NUMPADEQUALS;
	keymap[0x4B] = KC_DIVIDE;
	keymap[0x43] = KC_MULTIPLY;
	keymap[0x4C] = KC_NUMPADENTER;
	
	//Keypad with numlock off
	//keymap[0x73] = KC_NUMPAD7;  // not sure of these
	//keymap[XK_KP_Up] = KC_NUMPAD8; // check on a non-laptop
	//keymap[XK_KP_Page_Up] = KC_NUMPAD9;
	//keymap[XK_KP_Left] = KC_NUMPAD4;
	//keymap[XK_KP_Begin] = KC_NUMPAD5;
	//keymap[XK_KP_Right] = KC_NUMPAD6;
	//keymap[XK_KP_End] = KC_NUMPAD1;
	//keymap[XK_KP_Down] = KC_NUMPAD2;
	//keymap[XK_KP_Page_Down] = KC_NUMPAD3;
	//keymap[XK_KP_Insert] = KC_NUMPAD0;
	//keymap[XK_KP_Delete] = KC_DECIMAL;
	
	keymap[0x7E] = KC_UP;
	keymap[0x7D] = KC_DOWN;
	keymap[0x7B] = KC_LEFT;
	keymap[0x7C] = KC_RIGHT;
	
	keymap[0x74] = KC_PGUP;
	keymap[0x79] = KC_PGDOWN;
	keymap[0x73] = KC_HOME;
	keymap[0x77] = KC_END;
	
	//keymap[XK_Print] = KC_SYSRQ;		// ??
	//keymap[XK_Scroll_Lock] = KC_SCROLL; // ??
	//keymap[XK_Pause] = KC_PAUSE;		// ??
	
	
	//keymap[XK_Insert] = KC_INSERT;	  // ??
	keymap[0x75] = KC_DELETE; // del under help key?
}

- (void)windowWillClose:(NSNotification *)notification
{
    input->removeDevice( *keyboard );
//    delete keyboard;
    
	//shutdown the app and let the Platform continue
	[[NSApplication sharedApplication] stop:self];
}

- (void)stepCallback:(NSTimer *)timer
{
	CFTimeInterval deltaTime = CFAbsoluteTimeGetCurrent() - time;
	
	time = CFAbsoluteTimeGetCurrent();
	
	platform->step( deltaTime );
}

- (void)mouseDown: (NSEvent *)theEvent
{
    lastMousePos = [theEvent locationInWindow];    
    input->_fireTouchBeginEvent( Vector( lastMousePos.x, platform->getGame().getNativeHeight() - lastMousePos.y ), Touch::Type::LeftClick);
}

- (void)mouseDragged: (NSEvent *)theEvent
{
	NSPoint loc = [theEvent locationInWindow]; 
    
    Vector last( lastMousePos.x, platform->getGame().getNativeHeight() - lastMousePos.y );
    Vector cur( loc.x, platform->getGame().getNativeHeight() - loc.y );
    
    lastMousePos = loc;
    
	input->_fireTouchMoveEvent( last, cur, Touch::Type::LeftClick);
}

- (void)mouseUp: (NSEvent *)theEvent
{
	lastMousePos = [theEvent locationInWindow];    
	input->_fireTouchEndEvent( Vector( lastMousePos.x, platform->getGame().getNativeHeight() - lastMousePos.y ), Touch::Type::LeftClick);
}

- (void)mouseMoved: (NSEvent *)theEvent
{
	NSPoint loc = [theEvent locationInWindow]; 
    
    Vector last( lastMousePos.x, platform->getGame().getNativeHeight() - lastMousePos.y );
    Vector cur( loc.x, platform->getGame().getNativeHeight() - loc.y );
    
    lastMousePos = loc;
    
	input->_fireMouseMoveEvent( last, cur );
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
    keyboard->_notifyButtonState( keymap[kc], true );
}

- (void)keyUp:(NSEvent *)theEvent
{
	keyboard->_notifyButtonState( keymap[[theEvent keyCode]], false );
}

- (void)flagsChanged:(NSEvent *)theEvent
{
#define LEFT_MODIFIER (1<<8) //this was found disassembling modifier flags and is apparently true

    unsigned int mask = [theEvent modifierFlags];
    keyboard->_notifyButtonState( KC_LSHIFT, mask & NSEventModifierFlagShift
                                 && (mask & LEFT_MODIFIER) );
    keyboard->_notifyButtonState( KC_RSHIFT, mask & NSEventModifierFlagShift && (mask & LEFT_MODIFIER) == 0 );
    keyboard->_notifyButtonState( KC_LEFT_ALT, mask & NSEventModifierFlagOption && (mask & LEFT_MODIFIER) );
    keyboard->_notifyButtonState( KC_RIGHT_ALT, mask & NSEventModifierFlagOption && (mask & LEFT_MODIFIER) == 0 );
    keyboard->_notifyButtonState( KC_LCONTROL, mask & NSEventModifierFlagControl && (mask & LEFT_MODIFIER) );
    keyboard->_notifyButtonState( KC_RCONTROL, mask & NSEventModifierFlagControl && (mask & LEFT_MODIFIER) == 0 );
}

- (void)dealloc
{
    [super dealloc];
}

@end
