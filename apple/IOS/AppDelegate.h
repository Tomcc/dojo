//
//  NinjaAppDelegate.h
//  Ninja
//
//  Created by Tommaso Checchi on 4/23/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

#import <GameKit/GKLeaderboardViewController.h>

@class Application;

@interface AppDelegate : UIViewController <UIApplicationDelegate, GKLeaderboardViewControllerDelegate> {
    UIWindow *window;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet Application *glView;

- (void)showGameCenterLeaderboard;

- (void)leaderboardViewControllerDidFinish:(GKLeaderboardViewController *)viewController;

@end

