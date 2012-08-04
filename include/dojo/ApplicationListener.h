//
//  FocusListener.h
//
//  Created by Tommaso Checchi on 9/29/11.
//  Copyright (c) 2011 none. All rights reserved.
//

#ifndef Dojo_FocusListener_h
#define Dojo_FocusListener_h

#include "dojo_common_header.h"

///an interruption listener is able to listen interruptions to game's flow
namespace Dojo
{	
	class ApplicationListener
	{
	public:
		
		ApplicationListener()
		{
			
		}
		
		virtual ~ApplicationListener() {}
		
		///called in any non-fatal loss of focus case
		virtual void onApplicationFocusLost() {}			
		virtual void onApplicationFocusGained() {}
		
		///called on a "fatal" application freeze to serialize the current state
		virtual void onApplicationFreeze() {}
		///called when a freezed app is defreezed
		virtual void onApplicationDefreeze() {}			
		
		virtual void onApplicationTermination() {}
	};
}

#endif
