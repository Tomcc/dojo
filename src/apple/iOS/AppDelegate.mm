//
//  NinjaAppDelegate.m
//  Ninja
//
//  Created by Tommaso Checchi on 4/23/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "AppDelegate.h"
#import "Application.h"

#include "Game.h"
#include "Platform.h"

@implementation AppDelegate

@synthesize window;
@synthesize glView;

using namespace Dojo;

- (void) applicationDidFinishLaunching:(UIApplication *)application
{
	[glView initialise];
}

- (void) dealloc
{
	[window release];
	[glView release];
	
	[super dealloc];
}

- (void) applicationWillResignActive:(UIApplication *)application
{
	Platform::getSingleton()->_fireFocusLost();
	
	[glView stopAnimation];
}

- (void) applicationDidBecomeActive:(UIApplication *)application
{
	[glView startAnimation];
	
	Platform::getSingleton()->_fireFocusGained();
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
	[glView stopAnimation];	
	
	Platform::getSingleton()->_fireFreeze();
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{	
	Platform::getSingleton()->_fireDefreeze();
	
	[glView startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	[glView stopAnimation];
	
	Platform::getSingleton()->_fireTermination();
}

-(void)didReceiveMemoryWarning
{
	NSLog( @"MEMORY WARNING RECEIVED" );
	
	[super didReceiveMemoryWarning];
}

#ifdef GAME_CENTER_ENABLED

- (void)showGameCenterLeaderboard
{
	GKLeaderboardViewController *leaderboardController = [[GKLeaderboardViewController alloc] init];
    if (leaderboardController != nil)
    {
        leaderboardController.leaderboardDelegate = self;
        [self presentModalViewController: leaderboardController animated: YES];
    }
}

- (void)leaderboardViewControllerDidFinish:(GKLeaderboardViewController *)viewController
{
	[self dismissModalViewControllerAnimated:YES];
}

#endif

+ (void) _keepMeInLibraryFile
{
    //this method is required as without an un-optimizable method LLVM will remove this class as it is not called from any cpp file
    sleep(0);
}

@end
