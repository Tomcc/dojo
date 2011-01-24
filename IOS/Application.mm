//
//  EAGLView.m
//  Ninja
//
//  Created by Tommaso Checchi on 4/23/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "Application.h"

#include "Game.h"

using namespace Dojo;

@implementation Application

@synthesize animating;
@dynamic animationFrameInterval;

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
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
				
		[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
		
		game = NULL;
		renderImpl = NULL;
		touchSource = NULL;
		soundImpl = NULL;
		
		displayVisible = TRUE;
		
		animating = FALSE;
		displayLinkSupported = FALSE;
		animationFrameInterval = 60.f * Game::UPDATE_INTERVAL_CAP ;
		displayLink = nil;
		animationTimer = nil;
		
		// A system version of 3.1 or greater is required to use CADisplayLink. The NSTimer
		// class is used as fallback when it isn't available.
		NSString *reqSysVer = @"3.1";
		NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
		if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending)
			displayLinkSupported = TRUE;
    }
	
    return self;
}

- (void) dealloc
{	
	delete renderImpl;
	delete game;	
	
    [super dealloc];
} 

- (void) drawView:(id)sender
{		
	if( game ) //initialised?
	{	
		if( displayVisible )
		{
			//update the game and render game objects
			game->update( Game::UPDATE_INTERVAL_CAP );
			
			renderImpl->render();
			soundImpl->update( Game::UPDATE_INTERVAL_CAP );
		}
	}
}

- (void) layoutSubviews
{	
	CAEAGLLayer* layer = (CAEAGLLayer*)self.layer;
	
	// Set the scale factor to be the same as the main screen
	if ([self respondsToSelector: NSSelectorFromString(@"contentScaleFactor")]) {
		[self setContentScaleFactor:[[UIScreen mainScreen] scale]];
	}
		
	//set the needed render size	
	renderImpl->resizeFromLayer( layer );
	
    [self drawView:nil];
}

- (NSInteger) animationFrameInterval
{
	return animationFrameInterval;
}

- (void) setAnimationFrameInterval:(NSInteger)frameInterval
{
	// Frame interval defines how many display frames must pass between each time the
	// display link fires. The display link will only fire 30 times a second when the
	// frame internal is two on a display that refreshes 60 times a second. The default
	// frame interval setting of one will fire 60 times a second when the display refreshes
	// at 60 times a second. A frame interval setting of less than one results in undefined
	// behavior.
	if (frameInterval >= 1)
	{
		animationFrameInterval = frameInterval;
		
		if (animating)
		{
			[self stopAnimation];
			[self startAnimation];
		}
	}
}

- (BOOL) setup
{
	return false;
}


- (void) initialise
{
	//initialise accelerometer
	UIAccelerometer* accelerometer = [UIAccelerometer sharedAccelerometer];
	accelerometer.delegate = self;
	lastAccelerationX = lastAccelerationY = lastRoll = 0;
			
	//create C++ system
	renderImpl = new Render();
		
	if ( !renderImpl->isValid() )
	{
		[self release];
		return;
	}	
		
	soundImpl = new SoundManager();
	
	touchSource = new TouchSource();
	
	//call user setup method to create a game istance
	if( ![self setup] )
	{
		[self release];
		return;
	}
	
	touchSource = game;
	
	//animate
	[self startAnimation];
}

- (void) startAnimation
{
	[self becomeFirstResponder];
	
	if (!animating)
	{
		if (displayLinkSupported)
		{
			// CADisplayLink is API new to iPhone SDK 3.1. Compiling against earlier versions will result in a warning, but can be dismissed
			// if the system version runtime check for CADisplayLink exists in -initWithCoder:. The runtime check ensures this code will
			// not be called in system versions earlier than 3.1.

			displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
			[displayLink setFrameInterval:animationFrameInterval];
			[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		}
		else
			animationTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)( Game::UPDATE_INTERVAL_CAP ) target:self selector:@selector(drawView:) userInfo:nil repeats:TRUE];
		
		//notify openfeint
		//[OpenFeint applicationDidBecomeActive];
		
		game->onApplicationFocus();
		
		animating = TRUE;
	}
}

- (void)stopAnimation
{
	if (animating)
	{
		if (displayLinkSupported)
		{
			[displayLink invalidate];
			displayLink = nil;
		}
		else
		{
			[animationTimer invalidate];
			animationTimer = nil;
		}
				
		game->onApplicationFocusLost();
		
		animating = FALSE;
	}
}

- (BOOL)canBecomeFirstResponder
{ 
	return YES; 
}

Vector getInterfaceOrientatedPoint( int x, int y, Render* r )
{	
	x *= r->getContentScale();
	y *= r->getContentScale();
	
	int sy = r->getHeight();
	
	switch( r->getInterfaceOrientation() )
	{
		case Render::IO_PORTRAIT:
			return Vector( x, y );
			
		case Render::IO_PORTRAIT_REVERSE:
			return Vector( x, sy - y );
			
		case Render::IO_LANDSCAPE_RIGHT:		
			return Vector( y, sy - x );
			
		case Render::IO_LANDSCAPE_LEFT:			
			return Vector( y, x );
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
	CGPoint loc = [[[touches allObjects] objectAtIndex:0 ] locationInView:self ];
	
	Vector pos = getInterfaceOrientatedPoint( loc.x, loc.y, renderImpl );
	
	touchSource->_fireTouchMoveEvent( pos );
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

- (void)dashboardWillAppear
{
	game->onApplicationFocusLost();
}

- (void)dashboardDidAppear
{	
	displayVisible = false;
	soundImpl->pauseAll();
}

- (void)dashboardWillDisappear
{	
	displayVisible = true;
	
	soundImpl->resumeAll();
}

- (void)dashboardDidDisappear
{	
	game->onApplicationFocus();
}

- (void)userLoggedIn:(NSString*)userId
{
	game->onOpenFeintLogin();
}

@end
