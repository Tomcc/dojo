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

#include "Application.h"

namespace Dojo 
{
	class Game : public BaseObject, public StateInterface
	{
	public:
		
		static const float UPDATE_INTERVAL_CAP = 1.f/60.f;
				
		Game( Application* app, Render* r, SoundManager* s, TouchSource* t );
		
		virtual ~Game();
		
		inline Render* getRender()				{	return render;			}
		inline SoundManager* getSoundManager()	{	return soundManager;	}
		inline TouchSource* getTouchSource()	{	return touchSource;		}
		
		inline Application* getApplicationWrapper()
		{
			return application;
		}
		
		virtual void onBegin()=0;
		virtual void onLoop( float dt )=0;
		virtual void onEnd()=0;
		
		virtual void onApplicationFocus()=0;		
		virtual void onApplicationFocusLost()=0;
		
	protected:
		
		Application* application;
				
		Render* render;				
		SoundManager* soundManager;
		TouchSource* touchSource;
		
		bool focus;
	};
}

#endif