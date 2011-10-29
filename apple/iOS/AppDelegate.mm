//
//  NinjaAppDelegate.m
//  Ninja
//
//  Created by Tommaso Checchi on 4/23/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "AppDelegate.h"
#import "Application.h"

@implementation AppDelegate

@synthesize window;
@synthesize glView;

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
	[glView game]->_fireFocusLost();
	
	[glView stopAnimation];
}

- (void) applicationDidBecomeActive:(UIApplication *)application
{
	[glView startAnimation];
	
	[glView game]->_fireFocusGained();
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
	[glView stopAnimation];	
	
	[glView game]->_fireFreeze();
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{	
	[glView game]->_fireDefreeze();
	
	[glView startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	[glView stopAnimation];
	
	[glView game]->_fireTermination();
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

@end
