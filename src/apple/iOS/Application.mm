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
		platform->_initialiseImpl( self );
	
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

Vector getInterfaceOrientatedPoint( const CGPoint& pos, Render* r, UIView* v)
{
    int scale = [v contentScaleFactor];
    int x = pos.x * scale;
    int y = pos.y * scale;	
	int sx = [[UIScreen mainScreen] bounds].size.width * scale;
	int sy = [[UIScreen mainScreen] bounds].size.height * scale;
    	
	switch( r->getInterfaceOrientation() )
	{
		case DO_PORTRAIT:
			return Vector( x, y );
			
		case DO_PORTRAIT_REVERSE:
			return Vector( sx - x, sy - y );
			
		case DO_LANDSCAPE_RIGHT:		
			return Vector( y, sx - x );
			
		case DO_LANDSCAPE_LEFT:			
			return Vector( sy - y, x );
	}
	return Vector::ZERO;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	CGPoint loc;
		
	for( uint i = 0; i < [[touches allObjects] count]; ++i )
	{ 		
		loc = [[[touches allObjects] objectAtIndex:i ] locationInView:self ];
		
		Vector pos = getInterfaceOrientatedPoint( loc, renderImpl, self );	
		
		touchSource->_fireTouchBeginEvent( pos );		
	}	
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	CGPoint loc;
    CGPoint prevLoc;
	
	for( uint i = 0; i < [[touches allObjects] count]; ++i )
	{ 		
		loc = [[[touches allObjects] objectAtIndex:i ] locationInView:self ];
        prevLoc = [[[touches allObjects] objectAtIndex:i ] previousLocationInView:self ];
		
		Vector pos = getInterfaceOrientatedPoint( loc, renderImpl, self );
        Vector prevPos = getInterfaceOrientatedPoint( prevLoc, renderImpl, self );
		
		touchSource->_fireTouchMoveEvent( pos, prevPos );	
	}	
}	

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	CGPoint loc;
	
	for( uint i = 0; i < [[touches allObjects] count]; ++i )
	{ 		
		loc = [[[touches allObjects] objectAtIndex:i ] locationInView:self ];
				
		Vector pos = getInterfaceOrientatedPoint( loc, renderImpl, self );
		
		touchSource->_fireTouchEndEvent( pos );		
	}	
}

- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event
{	
	if (motion == UIEventSubtypeMotionShake)
		touchSource->_fireShakeEvent();
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
										Dojo::Vector( (float)acceleration.x,
										(float)acceleration.y,
										(float)acceleration.z ),
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
