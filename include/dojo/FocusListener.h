//
//  FocusListener.h
//
//  Created by Tommaso Checchi on 9/29/11.
//  Copyright (c) 2011 none. All rights reserved.
//

#ifndef Dojo_FocusListener_h
#define Dojo_FocusListener_h

///an interruption listener is able to listen interruptions to gameplay's flow
namespace Dojo
{	
	class FocusListener
	{
	public:
		
		FocusListener()
		{
			
		}
		
		virtual ~FocusListener() {}
		
		///called in any non-fatal loss of focus case
		virtual void onFocusLost() {}			
		virtual void onFocusGained() {}
		
		///called on a "fatal" application freeze to serialize the current state
		virtual void onFreeze() {}
		///called when a freezed app is defreezed
		virtual void onDefreeze() {}			
		
		virtual void onTermination() {}
	};
}

#endif
