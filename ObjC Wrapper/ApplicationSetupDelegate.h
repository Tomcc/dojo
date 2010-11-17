/*
 *  ApplicationSetupDelegate.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/4/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

//a game project must link an ApplicationSetupDelegate as the manager of the main View.
@protocol ApplicationSetupDelegate

- (BOOL) setup;

@end