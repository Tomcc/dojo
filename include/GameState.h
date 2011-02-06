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
#include "TouchSource.h"
#include "StateInterface.h"
#include "Renderable.h"

namespace Dojo {
	
	class Viewport;
	class SoundManager;
	class Renderable;
	class Object;
	class Game;
	
	class GameState : public ResourceGroup, public TouchSource::Listener, public Renderable::Listener, public StateInterface
	{
	public:
		
		typedef Array<Object*> ObjectList;
		typedef Array<Renderable*> RenderableList;
		
		GameState( Game* parentGame );
		
		virtual ~GameState();
				
		///clear and prepare for a new initialise
		virtual void clear();
		
		inline Game* getGame()				{	return game;			}
				
		inline float getCurrentTime()		{	return timeElapsed;		}		
		inline Viewport* getViewport()		{	return camera;			}
						
		inline void addClickable( Renderable* s )
		{
			clickables.add( s );
		}
		
		inline void addObject( Object* o )
		{
			objects.add( o );
		}
				
		void addObject( Renderable* s, int layer, bool clickable = false );
		
		inline void removeObject( Object* o )
		{
			objects.remove( o );
		}
		
		inline void removeClickableSprite( Renderable* s );
		void removeSprite( Renderable* s );
		
		void removeAll();
		
		Renderable* getClickableAtPoint( const Vector& point );

		void updateObjects( float dt );

		//metodi finalizzati qui perche' gamestate vede solo "clickables"
		void onTouchBegan( const Vector& point );
		void onTouchEnd( const Vector& point );
		
	protected:
		
		Game* game;
		
		Viewport* camera;
		
		float timeElapsed;		
		
		ObjectList objects;
		RenderableList clickables;
	};
}

#endif