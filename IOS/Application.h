//
//  EAGLView.h
//  Ninja
//
//  Created by Tommaso Checchi on 4/23/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import "ApplicationSetupDelegate.h"

#include "Render.h"
#include "SoundManager.h"
#include "TouchSource.h"
#include "Timer.h"
#include "Platform.h"

namespace Dojo {
	class Game;
}

// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
//#ifdef OPENFEINT_ENABLED
//@interface Application : UIView < UIAccelerometerDelegate, ApplicationSetupDelegate, OpenFeintDelegate >
//#elseif
@interface Application : UIView < UIAccelerometerDelegate, ApplicationSetupDelegate >
//#endif
{    
@protected
	Dojo::Render* renderImpl;	
	Dojo::Game* game;	
	Dojo::TouchSource* touchSource;
	Dojo::SoundManager* soundImpl;
	Dojo::Timer frameTimer;
	
	Dojo::Platform* platform;
	
@private
	float lastAccelerationX, lastAccelerationY, lastRoll;
	
	BOOL animating;
	BOOL displayLinkSupported;
	BOOL displayVisible;
	NSInteger animationFrameInterval;
	// Use of the CADisplayLink class is the preferred method for controlling your animation timing.
	// CADisplayLink will link to the main display and fire every vsync when added to a given run-loop.
	// The NSTimer class is used only as fallback when running on a pre 3.1 device where CADisplayLink
	// isn't available.
	id displayLink;
    NSTimer *animationTimer;
}

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;

- (void) initialise;
- (void) startAnimation;
- (void) stopAnimation;
- (void) drawView:(id)sender;

- (BOOL)canBecomeFirstResponder;

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event;

- (void)motionBegan:(UIEventSubtype)motion withEvent:(UIEvent *)event;
- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event;

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration;

- (void)dashboardWillAppear;
- (void)dashboardDidAppear;
- (void)dashboardWillDisappear;
- (void)dashboardDidDisappear;

- (void)userLoggedIn:(NSString*)userId;

@end
