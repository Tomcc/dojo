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
		
	protected:
		
		uint nativeWidth, nativeHeight;
		float mNativeFrameLength, mMaxFrameLength;
		
		Orientation nativeOrientation;

		String name;
	};
}

#endif