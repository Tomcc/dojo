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
		
		StateInterface() :
		currentState(-1),
		currentStatePtr( NULL )
		{
			
		}
		
		virtual ~StateInterface()
		{			
			if( currentStatePtr )
				delete currentStatePtr;
		}
		
		inline void setState( uint newState )		
		{
			_subStateEnd();
				
			currentState = newState;
			currentStatePtr = NULL;
				
			_subStateBegin();
		}
		
		inline void setState( StateInterface* child )
		{
			DEBUG_ASSERT( child );
			
			_subStateEnd();
			
			currentState = -1;
			currentStatePtr = child;
			
			_subStateBegin();			
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
		virtual void onStateBegin()=0;		
		virtual void onStateLoop( float dt )=0;		
		virtual void onStateEnd()=0;
				
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