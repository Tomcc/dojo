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
#include "StateInterface.h"
#include "ApplicationListener.h"

namespace Dojo 
{
	class Game : public StateInterface, public ApplicationListener
	{
	public:
			
		Game( const String& name, uint nativeWidth, uint nativeHeight, Orientation nativeOrientation, float nativeFrequency = 1.f/60.f, float maxFrameLength = 1.f/30.f );
		
		virtual ~Game();
		
		inline void addFocusListener( ApplicationListener* f )
		{
			DEBUG_ASSERT( f );
			DEBUG_ASSERT( !focusListeners.exists( f ) );
			
			focusListeners.add( f );
		}
		
		inline void removeFocusListener( ApplicationListener* f )
		{
			DEBUG_ASSERT( f  );
			
			focusListeners.remove( f );
		}

		inline const String& getName()
		{
			return name;	
		}
		
		inline uint getNativeWidth()
		{
			return nativeWidth;
		}
		
		inline uint getNativeHeight()
		{
			return nativeHeight;
		}
		
		inline float getNativeFrameLength()
		{
			return mNativeFrameLength;
		}
		
		inline float getMaximumFrameLength()
		{
			return mMaxFrameLength;
		}
		
		inline Orientation getNativeOrientation()
		{
			return nativeOrientation;
		}
		
		void _fireFocusLost()	{	for( int i = 0; i < focusListeners.size(); ++i )	focusListeners.at(i)->onApplicationFocusLost();	}
		void _fireFocusGained()	{	for( int i = 0; i < focusListeners.size(); ++i )	focusListeners.at(i)->onApplicationFocusGained();	}
		void _fireFreeze()		{	for( int i = 0; i < focusListeners.size(); ++i )	focusListeners.at(i)->onApplicationFreeze();	}
		void _fireDefreeze()	{	for( int i = 0; i < focusListeners.size(); ++i )	focusListeners.at(i)->onApplicationDefreeze();	}
		void _fireTermination()	{	for( int i = 0; i < focusListeners.size(); ++i )	focusListeners.at(i)->onApplicationTermination();	}
		
	protected:
		
		uint nativeWidth, nativeHeight;
		float mNativeFrameLength, mMaxFrameLength;
		
		Orientation nativeOrientation;
		
		Dojo::Array< ApplicationListener* > focusListeners;

		String name;
	};
}

#endif