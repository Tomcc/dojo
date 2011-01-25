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

#include "Application.h"

namespace Dojo {
	
	class GameState;
	class Object;
	
	class Game : public BaseObject, public TouchSource
	{
	public:
		
		static const float UPDATE_INTERVAL_CAP = 1.f/60.f;
				
		Game( Application* app, Render* r, SoundManager* s );
		
		virtual ~Game();
		
		inline Render* getRender()				{	return render;			}
		inline SoundManager* getSoundManager()	{	return soundManager;	}
		inline Application* getApplicationWrapper()
		{
			return application;
		}
		
		void setCurrentState( GameState* state );
				
		virtual void update( float dt );		
		
		virtual void onApplicationFocus()=0;
		
		virtual void onApplicationFocusLost()=0;
				
		virtual void onOpenFeintLogin()
		{
			//do nothing
		}
		
		///game works as inputManager, so it resends the callbacks it receives from iPhoneOS		
		virtual void _fireTouchBeginEvent( const Vector& point );		
		
		virtual void _fireTouchMoveEvent( const Vector& point );
		
		virtual void _fireTouchEndEvent( const Vector& point );		
		
	protected:
		
		Application* application;
				
		Render* render;	
		
		GameState* currentState;
		
		SoundManager* soundManager;
		
		bool focus;
	};
}

#endif