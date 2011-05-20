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

#include "dojo_common_header.h"

#include "Array.h"

#include "Render.h"
#include "InputSystem.h"
#include "SoundManager.h"
#include "InputSystem.h"
#include "StateInterface.h"

#include "Platform.h"

#include "Render.h"

namespace Dojo 
{
	class Game : public BaseObject, public StateInterface
	{
	public:
		
		static const float UPDATE_INTERVAL_CAP;
				
		Game( const std::string& name, uint nativeWidth, uint nativeHeight, Render::RenderOrientation nativeOrientation );
		
		virtual ~Game();

		inline const std::string& getName()
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

		std::string name;
	};
}

#endif