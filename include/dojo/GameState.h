/*
 *  GameState.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 5/16/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "Object.h"
#include "ResourceGroup.h"
#include "StateInterface.h"

namespace Dojo 
{	
	class Viewport;
	class SoundManager;
	class Game;
    class TouchArea;
	class Touch;

	///GameState is the Dojo's Level class
	/**
	each GameState is in itself an Object, and represents the root node of its Object tree;
	it is also a ResourceGroup, meaning that a GameState can be associated easily with the Resources it loads;
	and it is implements StateInterface, so that it can easily become a substate of Game, and manage its own (often complex) internal states.
	*/
	class GameState : 
		public Object, 
		public ResourceGroup, 
		public StateInterface
	{
	public:
		
		///Creates a new GameState with a parent Game
		GameState( Game* parentGame );
		
		virtual ~GameState();
        
		///Registers an existing TouchArea in this GameState
        void addTouchArea( TouchArea* t );

        ///Unregisters an existing TouchArea in this GameState
        void removeTouchArea( TouchArea* t );
				
		///Clears this GameState to a pre-initialization state
		virtual void clear();
		
		///returns the parent Game
		Game* getGame()				{	return game;			}
				
		///returns the Viewport that is primary on this GameState
		Viewport* getViewport()		{	return camera;			}
		
		///sets the primary Viewport (ie. camera) on this GameState, needed for pixel-perfect behaviour! (Sprites and TextAreas)
		void setViewport( Viewport& v );
		
		///"touches" all the touchAreas with the given touch
		/**touched TouchAreas will fire onTouchAreaPressed() on their listeners as soon as updateClickableState() is called*/
		void touchAreaAtPoint( const Touch& touch );        

		///triggers all the TouchAreas to send their events if they were touched before the last updateClickableState call
        void updateClickableState();
        
		///default implementation for the GameState, with TouchAreas and child objects update
		/**
		\remark remember to call GameState::onLoop() if you override this method!
		*/
        virtual void onLoop( float dt )
        {
            updateClickableState();
            
            updateChilds( dt );
        }
		
	protected:
        
        typedef std::vector< TouchArea* > TouchAreaList;
        
        TouchAreaList mTouchAreas;
		
		Game* game;
		
		Viewport* camera;
		
		float timeElapsed;
	};
}

