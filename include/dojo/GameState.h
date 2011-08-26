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

namespace Dojo {
	
	class Viewport;
	class SoundManager;
	class Renderable;
	class Object;
	class Game;
	
	class GameState : public ResourceGroup, public InputSystem::Listener, public Renderable::Listener, public StateInterface
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
			DEBUG_ASSERT( s );
			
			clickables.add( s );
		}
		
		inline void addObject( Object* o )
		{
			DEBUG_ASSERT( o );
			
			objects.add( o );
		}
		
		void addLight( Light* l );
				
		void addObject( Renderable* s, int layer, bool clickable = false );
		
		void setViewport( Viewport* v );
		
		inline void removeObject( Object* o )
		{
			objects.remove( o );
		}
		
		void removeClickableSprite( Renderable* s );
        
		void removeSprite( Renderable* s );
		
		void removeAll();

		inline void destroyAll()
		{
			for( uint i = 0; i < objects.size(); ++i )
				delete objects.at(i);

			//clear vectors
			removeAll();
		}
		
		Renderable* getClickableAtPoint( const Vector& point );

		void updateObjects( float dt );

		virtual void onTouchBegan( const Vector& point );
		virtual void onTouchEnd( const Vector& point );
		
	protected:
		
		Game* game;
		
		Viewport* camera;
		
		float timeElapsed;		
		
		ObjectList objects;
		RenderableList clickables;
	};
}

#endif