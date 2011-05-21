/*
 *  StateInterface.h
 *  Dojo Training
 *
 *  Created by Tommaso Checchi on 5/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef StateInterface_h__
#define StateInterface_h__

#include "dojo_common_header.h"

namespace Dojo
{
	class StateInterface
	{
	public:
		
		bool autoDeleteWhenReplaced;
		
		StateInterface() :
		currentState(-1),
		currentStatePtr( NULL ),
		autoDeleteWhenReplaced( true )
		{
			
		}
		
		virtual ~StateInterface()
		{			
			if( currentStatePtr )
				delete currentStatePtr;
		}
		
		inline void setState( uint newState )		
		{
			_setState( newState, NULL );
		}
		
		inline void setState( StateInterface* child )
		{
			DEBUG_ASSERT( child );
			
			_setState( -1, child );
		}
		
		inline uint getCurrentState()					{	return currentState;	}
		inline StateInterface* getChildState()		{	return currentStatePtr;	}
		
		inline bool isCurrentState( uint state )		{	return currentState == state;	}
		inline bool isCurrentState( StateInterface* s )	{	return currentStatePtr == s;	}

		inline bool hasChildState()						{	return currentStatePtr != NULL;	}
		
		///begin the execution of this state
		void begin()
		{
			onBegin();
		}
		
		///loop the execution of this state (and its childs)
		void loop( float dt )
		{					
			onLoop( dt );
			
			_subStateLoop( dt );
		}
		
		///end the execution of this state (and its childs)
		void end()
		{
			_subStateEnd();
			
			onEnd();	
		}
						
	protected:
		int currentState;		
		StateInterface* currentStatePtr;
		
	private:
				
		//------ eventi dello stato
		virtual void onBegin()
		{
			
		}
		virtual void onLoop( float dt )
		{
			
		}
		virtual void onEnd()
		{
			
		}
		
		//----- eventi per i sottostati immediati
		virtual void onStateBegin()
		{
			
		}	
		
		virtual void onStateLoop( float dt )
		{
			
		}
		
		virtual void onStateEnd()
		{
			
		}
		
		void _setState( int newState, StateInterface* newChild )
		{
			_subStateEnd();
			
			//kill old state?
			/*if( hasChildState() && currentStatePtr->autoDeleteWhenReplaced )
				delete currentStatePtr;*/
			
			currentState = newState;
			currentStatePtr = newChild;
			
			_subStateBegin();	
		}
		
		inline void _subStateBegin()
		{
			if( currentStatePtr )
				currentStatePtr->begin();
			else
				onStateBegin();
		}
		
		inline void _subStateLoop( float dt )
		{
			if( currentStatePtr )
				currentStatePtr->loop( dt );
			else
				onStateLoop( dt );
		}
		
		inline void _subStateEnd()
		{
			if( currentStatePtr )
				currentStatePtr->end();
			else
				onStateEnd();
		}
		
	};
}

#endif