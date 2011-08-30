/*
 *  GameState.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 5/16/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef GameState_h__
#define GameState_h__

#include "dojo_common_header.h"

#include "Array.h"

#include "ResourceGroup.h"
#include "InputSystem.h"
#include "StateInterface.h"
#include "Renderable.h"
#include "Light.h"
#include "Object.h"

namespace Dojo {
	
	class Viewport;
	class SoundManager;
	class Renderable;
	class Game;
	
	class GameState : public Object, public ResourceGroup, public InputSystem::Listener, public Renderable::Listener, public StateInterface
	{
	public:
		
		GameState( Game* parentGame );
		
		virtual ~GameState();
				
		///clear and prepare for a new initialise
		virtual void clear();
		
		inline Game* getGame()				{	return game;			}
				
		inline float getCurrentTime()		{	return timeElapsed;		}		
		inline Viewport* getViewport()		{	return camera;			}
		
		void setViewport( Viewport* v );
		
		Renderable* getClickableAtPoint( const Vector& point );

		virtual void onTouchBegan( const Vector& point );
		virtual void onTouchEnd( const Vector& point );
		
	protected:
		
		Game* game;
		
		Viewport* camera;
		
		float timeElapsed;
	};
}

#endif