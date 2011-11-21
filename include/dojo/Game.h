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
#include "FocusListener.h"

namespace Dojo 
{
	class Game : public StateInterface, public FocusListener
	{
	public:
			
		Game( const String& name, uint nativeWidth, uint nativeHeight, Render::RenderOrientation nativeOrientation, float nativeFrequency = 1.f/60.f, float maxFrameLength = 1.f/30.f );
		
		virtual ~Game();
		
		inline void addFocusListener( FocusListener* f )
		{
			DEBUG_ASSERT( f );
			DEBUG_ASSERT( !focusListeners.exists( f ) );
			
			focusListeners.add( f );
		}
		
		inline void removeFocusListener( FocusListener* f )
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
		
		inline Render::RenderOrientation getNativeOrientation()
		{
			return nativeOrientation;
		}
		
		void _fireFocusLost()	{	for( int i = 0; i < focusListeners.size(); ++i )	focusListeners.at(i)->onFocusLost();	}
		void _fireFocusGained()	{	for( int i = 0; i < focusListeners.size(); ++i )	focusListeners.at(i)->onFocusGained();	}
		void _fireFreeze()		{	for( int i = 0; i < focusListeners.size(); ++i )	focusListeners.at(i)->onFreeze();	}
		void _fireDefreeze()	{	for( int i = 0; i < focusListeners.size(); ++i )	focusListeners.at(i)->onDefreeze();	}
		void _fireTermination()	{	for( int i = 0; i < focusListeners.size(); ++i )	focusListeners.at(i)->onTermination();	}
		
	protected:
		
		uint nativeWidth, nativeHeight;
		float mNativeFrameLength, mMaxFrameLength;
		
		Render::RenderOrientation nativeOrientation;
		
		Dojo::Array< FocusListener* > focusListeners;

		String name;
	};
}

#endif