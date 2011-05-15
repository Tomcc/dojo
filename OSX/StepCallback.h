//
//  StepCallback.h
//  dojo
//
//  Created by Tommaso Checchi on 5/15/11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <CoreFoundation/CoreFoundation.h>

namespace Dojo
{
	class Platform;
}

@interface StepCallback : NSObject {
@private
	Dojo::Platform* platform;
	CFAbsoluteTime time;
    
}

- (id)initWithPlatform:(Dojo::Platform*)targetPlatform;

- (void) stepCallback:(NSTimer *)timer;

@end
