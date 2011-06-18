/*
 *  Game.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef Game_h__
#define Game_h__

#include "dojo/dojo_common_header.h"

#include "dojo/Array.h"

#include "dojo/Render.h"
#include "dojo/InputSystem.h"
#include "dojo/SoundManager.h"
#include "dojo/InputSystem.h"
#include "dojo/StateInterface.h"

#include "dojo/Platform.h"

#include "dojo/Render.h"

namespace Dojo 
{
	class Game : public BaseObject, public StateInterface
	{
	public:
		
		static const float UPDATE_INTERVAL_CAP;
				
		Game( const String& name, uint nativeWidth, uint nativeHeight, Render::RenderOrientation nativeOrientation );
		
		virtual ~Game();

		inline const String& getName()
		{
			return name;	
		}
		
		inline uint getNativeWidth()
		{
			return nativeWidth;
		}
		
		inline uint getNativeHeight()
		{
			return nativeHeight;
		}
		
		inline Render::RenderOrientation getNativeOrientation()
		{
			return nativeOrientation;
		}
								
		virtual void onApplicationFocus()=0;		
		virtual void onApplicationFocusLost()=0;
		
	protected:		
		bool focus;
		
		uint nativeWidth, nativeHeight;
		
		Render::RenderOrientation nativeOrientation;

		String name;
	};
}

#endif