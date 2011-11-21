/*
 *  Sprite.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef Sprite_h__
#define Sprite_h__

#include "dojo_common_header.h"

#include "Array.h"

#include "AnimatedQuad.h"

namespace Dojo
{	
	class Sprite : public AnimatedQuad
	{	
		
	public:		
				
		typedef Array<Animation*> AnimationList;
														
		Sprite( GameState* level, const Vector& pos, const String& defaultAnimName = String::EMPTY, float timePerFrame = 1, bool pixelPerfect = true );
				
		virtual ~Sprite()
		{
			for( uint i = 0; i < animations.size(); ++i )
				SAFE_DELETE( animations.at(i) );
						
			//frames have to be relased manually from the group!
		}		

		virtual void reset()
		{
			AnimatedQuad::reset();

			if( animations.size() )
				setAnimation(0);
		}

		///registers the given animation and returns its handle
		inline int registerAnimation( FrameSet* set, float timePerFrame )
		{
			DEBUG_ASSERT( set );
						
			Animation* a = new Animation( set, timePerFrame );
		
			animations.add( a );		
			
			//if no current animation, set this as default
			if( animations.size() == 1 )
				setAnimation( 0 );
			
			return animations.size()-1;
		}
		
		int registerAnimation( const String& base, float timePerFrame );
				
		///sets the animation at the given index
		inline void setAnimation( int i ) 	
		{				
			DEBUG_ASSERT( i >= 0 );
			DEBUG_ASSERT( animations.size() > i );
			
			animation = animations.at(i);
			
			_setTexture( animation->getCurrentFrame() );
			
			_updateScreenSize();
		}
		
		///returns the current animation index. TODO don't use a linear search
		inline int getAnimation()
		{
			return animations.getElementIndex( animation );
		}
		
	protected:		
		AnimationList animations;
	};
}

#endif