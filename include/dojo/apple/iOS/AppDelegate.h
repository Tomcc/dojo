//
//  NinjaAppDelegate.h
//  Ninja
//
//  Created by Tommaso Checchi on 4/23/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "Application.h"

#ifdef GAME_CENTER_ENABLED
#import <GameKit/GKLeaderboardViewController.h>
@interface AppDelegate : UIViewController <UIApplicationDelegate, GKLeaderboardViewControllerDelegate> {
#else
@interface AppDelegate : UIViewController <UIApplicationDelegate > {
#endif

	UIWindow* window;
}

@property (nonatomic, retain) IBOutlet UIWindow* window;
@property (nonatomic, retain) IBOutlet Application* glView;

#ifdef GAME_CENTER_ENABLED
- (void)showGameCenterLeaderboard;

- (void)leaderboardViewControllerDidFinish:(GKLeaderboardViewController*)viewController;
#endif

+ (void) _keepMeInLibraryFile;

@end

