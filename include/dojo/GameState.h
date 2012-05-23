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

#include "Object.h"
#include "ResourceGroup.h"
#include "InputSystem.h"
#include "Renderable.h"
#include "StateInterface.h"

namespace Dojo 
{	
	class Viewport;
	class SoundManager;
	class Game;
    class TouchArea;
	
	class GameState : public Object, public ResourceGroup, public StateInterface
	{
	public:
		
		GameState( Game* parentGame );
		
		virtual ~GameState();
        
        inline void addTouchArea( TouchArea* t )
        {
            DEBUG_ASSERT( t );
            
            mTouchAreas.add( t );
        }
        
        inline void removeTouchArea( TouchArea* t )
        {
            DEBUG_ASSERT( t );
            
            mTouchAreas.remove( t );
        }
				
		///clear and prepare for a new initialise
		virtual void clear();
		
		inline Game* getGame()				{	return game;			}
				
		inline float getCurrentTime()		{	return timeElapsed;		}		
		inline Viewport* getViewport()		{	return camera;			}
		
		void setViewport( Viewport* v );
		
		void touchAreaAtPoint( const Vector& point );        
        void updateClickableState();
        
        virtual void onLoop( float dt )
        {
            updateClickableState();
            
            updateChilds( dt );
        }
		
	protected:
        
        typedef Array< TouchArea* > TouchAreaList;
        
        TouchAreaList mTouchAreas;
		
		Game* game;
		
		Viewport* camera;
		
		float timeElapsed;
	};
}

#endif