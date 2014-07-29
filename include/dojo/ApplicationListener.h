//
//  FocusListener.h
//
//  Created by Tommaso Checchi on 9/29/11.
//  Copyright (c) 2011 none. All rights reserved.
//

#pragma once

#include "dojo_common_header.h"

namespace Dojo
{	
	///ApplicationListener receives events about the application state from the Platform
	class ApplicationListener
	{
	public:
		
		ApplicationListener()
		{
			
		}
		
		virtual ~ApplicationListener() {}
		
		///called in any non-fatal loss of focus case, eg: switched window, a modal popup appeared
		virtual void onApplicationFocusLost() {}

		///called when the focus is regained, eg: switched window back, modal popup disappeared
		virtual void onApplicationFocusGained() {}
		
		///called when the application is set into "background" state and should stop updating
		/** 
		\remark the application can be terminated WITHOUT NOTICE from now on! */
		virtual void onApplicationFreeze() {}

		///called when a previously freezed app is "defreezed" into a state where it can update again
		virtual void onApplicationDefreeze() {}
		
		///called when an app is closed by the user. Collection from background state WILL NOT send onApplicationTermination
		virtual void onApplicationTermination() {}
	};
}

