//
//  TouchArea.h
//  dojo
//
//  Created by Tommaso Checchi on 5/23/12.
//  Copyright (c) 2012 none. All rights reserved.
//

#pragma once

#include "dojo_common_header.h"

#include "Object.h"
#include "Touch.h"

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

		typedef std::vector< Touch > TouchList;
        
        class Listener
        {
        public:
            
			///Tapped is sent when the TA was previously "free", and then a touch starts inside the area
			/**
			differs with Pressed as it won't be sent, for example, when the user drags a finger inside the area
			*/
			virtual void onTouchAreaTapped(TouchArea& area) {}

			///Pressed is sent when a previously untouched area is tapped
			virtual void onTouchAreaPressed(TouchArea& area)     {}

			///Released is sent when a previously touched (one or more touches) is released inside this area
			virtual void onTouchAreaReleased(TouchArea& area)	{}
            
        protected:
        };
        
        ///creates a new "free touch area" with the given parameters
        TouchArea( Listener* l, Object* parent, const Vector& pos, const Vector& size, int layer = 0 );
        
        ///creates a new touch area copying the renderable's parameters
        TouchArea( Renderable* image, Listener* l );
        
        virtual ~TouchArea();
                
        void setListener( Listener* l );
        
        int getLayer() const;

		bool isTop() const {
			return top;
		}

		void setTop(bool enable) {
			top = enable;
		}

		///returns a list of the touches that entered this toucharea in the last frame
		const TouchList& getTouchList() const;

		///tells if this area currently contains at least one touch
		bool isPressed() const;
                
        void _fireOnTouchUsingCurrentTouches();
        
        void _notifyLayer( int l );

		void _clearTouches();

		void _incrementTouches( const Touch& touch );
        
    protected:        
        bool mPressed, top = false;
        int mLayer;

		TouchList mTouches;
        
        Listener* listener;

		
    };
}

