//
//  NinjaAppDelegate.h
//  Ninja
//
//  Created by Tommaso Checchi on 4/23/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

@class Application;

@interface AppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    Application *glView;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet Application *glView;

@end

