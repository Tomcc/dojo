//
//  TouchArea.h
//  dojo
//
//  Created by Tommaso Checchi on 5/23/12.
//  Copyright (c) 2012 none. All rights reserved.
//

#ifndef dojo_TouchArea_h
#define dojo_TouchArea_h

#include "dojo_common_header.h"

#include "Object.h"

namespace Dojo
{
    class GameState;
    class Renderable;
    
	///A TouchArea is an Object used as the basic building block for UI buttons
	/**
	When added to a GameState, a TouchArea will send Pressed and Released events to its listeners,
	each time that the touch count on it change from 0 to one or more.
	
	Touching again an already touched TouchArea *will not* spawn new Pressed and Released events.
	*/
    class TouchArea : public Object
    {
    public:
        
        class Listener
        {
        public:
            
			///Pressed is sent when a previously untouched area is tapped
            virtual void onTouchAreaPressed( Dojo::TouchArea* )     {}	
			///Released is sent when a previously touched (one or more touches) is released
            virtual void onTouchAreaReleased( Dojo::TouchArea* )	{}
            
        protected:            
        };
        
        ///creates a new "free touch area" with the given parameters
        TouchArea( Listener* l, Dojo::GameState* state, const Vector& pos, const Vector& size, int layer = 0 );
        
        ///creates a new touch area copying the renderable's parameters
        TouchArea( Renderable* image, Listener* l );
        
        virtual ~TouchArea();
                
        inline void setListener( Listener* l )
        {
            DEBUG_ASSERT( l );
    
            listener = l;
        }
        
        inline int getLayer()
        {
            return mLayer;
        }
        
        inline void _incrementTouches()
        {
            ++mTouches;
        }
                
        inline void _fireOnTouchUsingCurrentTouches()
        {
            //set the state to clicked if there's at least one touch
            bool active = mTouches > 0;
            
            if( mPressed != active ) ///fire the event on state change
            {                
                mPressed = active;
                
                if( mPressed )      listener->onTouchAreaPressed( this );
                else                listener->onTouchAreaReleased( this );
            }
            
            mTouches = 0;
        }
        
        inline void _notifyLayer( int l )
        {
            mLayer = l;
        }
        
    protected:        
        bool mPressed;
        int mTouches, mLayer;
        
        Listener* listener;
    };
}

#endif
