//
//  EAGLView.m
//  Ninja
//
//  Created by Tommaso Checchi on 4/23/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "Application.h"

#include "Game.h"

#include "Render.h"
#include "SoundManager.h"
#include "InputSystem.h"

#include "IOSPlatform.h"

using namespace Dojo;

@implementation Application

@synthesize animating;

// You must implement this method
+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

//The GL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id) initWithCoder:(NSCoder*)coder
{    
    if ((self = [super initWithCoder:coder]))
	{
       
    }
	
    return self;
}

- (void) dealloc
{		
    [super dealloc];
} 

- (void) drawView:(id)sender
{		
	platform->step( frameTimer.deltaTime() ); //one step
}

- (void) layoutSubviews
{		
	//set the scale factor to 2 to support retina display, if available
	
	bool hasRetina = ([[UIScreen mainScreen] respondsToSelector:@selector(scale)] && [[UIScreen mainScreen] scale] == 2);
	
	if ( hasRetina && [self respondsToSelector: NSSelectorFromString(@"contentScaleFactor")])
		[self setContentScaleFactor:2];
	
	if( !renderImpl )
	{
		platform->initialise();
	
		renderImpl = platform->getRender();
		touchSource = platform->getInput();	
		
		frameTimer.reset();
	}
}

- (void) initialise
{
	// Get the layer
	CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
	
	eaglLayer.opaque = TRUE;
	eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
									[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
	
	[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	
	displayVisible = TRUE;
	
	animating = FALSE;
	displayLink = nil;
	renderImpl = NULL;

	//initialise accelerometer
	UIAccelerometer* accelerometer = [UIAccelerometer sharedAccelerometer];
	accelerometer.delegate = self;
	lastAccelerationX = lastAccelerationY = lastRoll = 0;
			
	//get C++ system
	platform = (IOSPlatform*)Platform::getSingleton();
	
	platform->_notifyNativeApp( self );
}

- (void) startAnimation
{	
	if (!animating)
	{
		displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
		
		[displayLink setFrameInterval: platform->getGame()->getNativeFrameLength() ];
		[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		
		animating = TRUE;
	}
	//reset frametimer to avoid huge time differences after restoring!
	frameTimer.reset();

	[self becomeFirstResponder];
}

- (Dojo::Game*) game
{
	return platform->getGame();
}

- (void)stopAnimation
{
	if (animating)
	{
		[displayLink invalidate];
		displayLink = nil;
			
		animating = FALSE;
	}
	
	frameTimer.reset();
}

- (BOOL)canBecomeFirstResponder
{ 
	return YES; 
}

Vector getInterfaceOrientatedPoint( int x, int y, Render* r );

Vector getInterfaceOrientatedPoint( int x, int y, Render* r )
{		
	x *= r->getContentScale();
	y *= r->getContentScale();
	
	int sx = Platform::getSingleton()->getGame()->getNativeWidth();
	int sy = Platform::getSingleton()->getGame()->getNativeHeight();
	
	switch( r->getInterfaceOrientation() )
	{
		case Render::RO_PORTRAIT:
			return Vector( x, y );
			
		case Render::RO_PORTRAIT_REVERSE:
			return Vector( sx - x, sy - y );
			
		case Render::RO_LANDSCAPE_RIGHT:		
			return Vector( y, sy - x );
			
		case Render::RO_LANDSCAPE_LEFT:			
			return Vector( sx - y, x );
	}
	return Vector::ZERO;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	CGPoint loc;
		
	for( uint i = 0; i < [[touches allObjects] count]; ++i )
	{ 		
		loc = [[[touches allObjects] objectAtIndex:i ] locationInView:self ];
		
		Vector pos = getInterfaceOrientatedPoint( loc.x, loc.y, renderImpl );	
		
		touchSource->_fireTouchBeginEvent( pos );		
	}	
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	CGPoint loc;
	
	for( uint i = 0; i < [[touches allObjects] count]; ++i )
	{ 		
		loc = [[[touches allObjects] objectAtIndex:i ] locationInView:self ];
		
		Vector pos = getInterfaceOrientatedPoint( loc.x, loc.y, renderImpl );	
		
		touchSource->_fireTouchMoveEvent( pos );	
	}	
}	

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	CGPoint loc;
	
	for( uint i = 0; i < [[touches allObjects] count]; ++i )
	{ 		
		loc = [[[touches allObjects] objectAtIndex:i ] locationInView:self ];
				
		Vector pos = getInterfaceOrientatedPoint( loc.x, loc.y, renderImpl );
		
		touchSource->_fireTouchEndEvent( pos );		
	}	
}

- (void)motionBegan:(UIEventSubtype)motion withEvent:(UIEvent *)event
{	
	if (motion == UIEventSubtypeMotionShake)
		touchSource->_fireShakeEvent();
}

- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
	
}

#define kFilteringFactor 0.05

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration
{		
	// Use a basic low-pass filter to only keep the gravity in the accelerometer values for the X and Y axes
    lastAccelerationX = (float)acceleration.x * kFilteringFactor + lastAccelerationX * (1.0 - kFilteringFactor);
    lastAccelerationY = (float)acceleration.y * kFilteringFactor + lastAccelerationY * (1.0 - kFilteringFactor);
    		
	float relativeRoll = (lastAccelerationY - lastRoll);
	lastRoll = lastAccelerationY;
	
	touchSource->_fireAccelerationEvent( 
										(float)acceleration.x,
										(float)acceleration.y,
										(float)acceleration.z,
										relativeRoll);
}
/*
- (void)mailComposeController:(MFMailComposeViewController*)controller didFinishWithResult:(MFMailComposeResult)result error:(NSError*)error
{
	platform->onEmailSent( controller, result != MFMailComposeResultFailed );
		
	[[controller view] resignFirstResponder];
}
*/

@end
