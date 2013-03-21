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
	///A Sprite is a wrapper around AnimatedQuad providing management for multiple registered Animations
	class Sprite : public AnimatedQuad
	{	
		
	public:
				
		typedef Array<Animation*> AnimationList;
								
		///Creates a new Sprite in a GameState, at position Pos, with the given default Animation (which must exist!)
		/**
			\param level the level it will be created in
			\param pos its position
			\param defaultAnimName the name of animation 0
			\param timePerFrame the frame time of the animation
			\param pixelPerfect if pixelPerfect, an objects' scale is bound to the pixel size of the current frame and to the pixel size of the current Viewport. A pixelPerfect object can still be scaled using pixelScale.*/
		Sprite( GameState* level, const Vector& pos, const String& defaultAnimName = String::EMPTY, float timePerFrame = -1, bool pixelPerfect = true );
				
		virtual ~Sprite()
		{
			for( int i = 0; i < animations.size(); ++i )
				SAFE_DELETE( animations.at(i) );
						
			//frames have to be released manually from the group!
		}		

		virtual void reset()
		{
			AnimatedQuad::reset();

			if( animations.size() )
				setAnimation(0);
		}

		///registers the given animation and returns its handle
		/**
		If timePerFrame is not specified, the animation tries to use the frameset's preferred
		*/
		inline int registerAnimation( FrameSet* set, float timePerFrame = -1 )
		{
			DEBUG_ASSERT( set != nullptr, "registering a null frameset" );

			if( timePerFrame < 0 )
				timePerFrame = set->getPreferredAnimationTime();

			DEBUG_ASSERT( timePerFrame >= 0, "the time per frame of an animation can't be negative" );
						
			Animation* a = new Animation( set, timePerFrame );
		
			animations.add( a );		
			
			//if no current animation, set this as default
			if( mAnimationIdx == -1 )
				setAnimation( 0 );
			
			return animations.size()-1;
		}
		
		///registers an animation using the FrameSet named base
		int registerAnimation( const String& base, float timePerFrame = -1 );
				
		///sets the animation at the given index
		inline void setAnimation( int i ) 	
		{
			mAnimationIdx = i;

			DEBUG_ASSERT( mAnimationIdx >= 0, "negative animation index" );
			DEBUG_ASSERT( animations.size() > mAnimationIdx, "OOB animation index" );
			
			if( animation )
				animation->_unset();

			animation = animations.at( mAnimationIdx );
			
			_setTexture( animation->getCurrentFrame() );
			
			_updateScreenSize();
		}

		///returns the current animation Index
		inline int getAnimationIndex()
		{
			return mAnimationIdx;
		}
		
	protected:		
		AnimationList animations;

		int mAnimationIdx;
	};
}

#endif