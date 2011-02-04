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
#include "TouchSource.h"
#include "SoundManager.h"
#include "TouchSource.h"
#include "StateInterface.h"

#include "Platform.h"

namespace Dojo 
{
	class Game : public BaseObject, public StateInterface
	{
	public:
		
		static const float UPDATE_INTERVAL_CAP;
				
		Game();
		
		virtual ~Game();
		
		inline Render* getRender()				{	return platform->getRender();		}
		inline SoundManager* getSoundManager()	{	return platform->getSoundManager();	}
		inline TouchSource* getTouchSource()	{	return platform->getInput();		}
				
		virtual void onBegin()=0;
		virtual void onLoop( float dt )=0;
		virtual void onEnd()=0;
		
		virtual void onApplicationFocus()=0;		
		virtual void onApplicationFocusLost()=0;
		
	protected:
		
		Platform* platform;
		
		bool focus;
	};
}

#endif