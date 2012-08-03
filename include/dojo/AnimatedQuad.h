/*
 *  AnimatedQuad.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/13/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#ifndef AnimatedQuad_h__
#define AnimatedQuad_h__

#include "dojo_common_header.h"

#include "Renderable.h"
#include "FrameSet.h"

namespace Dojo
{	
	class AnimatedQuad : public Renderable
	{
	protected:
		
		class Animation
		{
		public:		
			
			FrameSet* frames;
			
			Animation( FrameSet* set, float timePerFrame ) :
			currentFrame( NULL ),
			mElapsedLoops(0)
			{
				setup( set, timePerFrame );
			}			
						
			inline void setup( FrameSet* set, float tpf )
			{
				DEBUG_ASSERT( tpf >= 0 );
				
				animationTime = 0;
				timePerFrame = tpf;			
				frames = set;
				
				if( frames )
				{
					totalTime = timePerFrame * frames->getFrameNumber();
					
					if( frames->getFrameNumber() > 0 )
						currentFrame = frames->getFrame( 0 );
				}
				else
					totalTime = 1;
			}
			
			inline Texture* getCurrentFrame()
			{					
				return currentFrame;
			}			
			
			inline int getCurrentFrameNumber()
			{
				return frames->getFrameIndex( currentFrame );
			}
			
			inline float getTimePerFrame()
			{
				return timePerFrame;
			}
			inline float getTotalTime()
			{
				return totalTime;
			}

			///returns how many loops have elapsed since the last setup
			inline int getElapsedLoops()
			{
				return mElapsedLoops;
			}
			
			inline void setFrame( uint i )
			{
				DEBUG_ASSERT( frames );
				DEBUG_ASSERT( frames->getFrameNumber() > i );
				
				currentFrame = frames->getFrame( i );
				
				animationTime = i * timePerFrame;
			}
			
			inline void setAnimationTime( float t )
			{				
				DEBUG_ASSERT( frames );
				
				if( timePerFrame == 0 )
					return;
				
				if( frames->getFrameNumber() <= 1 ) //can't set time on a void or one-frame animation
					return;
								
				animationTime = t;
				
				//clamp in the time interval
				while( animationTime >= totalTime )
				{
					++mElapsedLoops;
					animationTime -= totalTime;
				}
				
				while( animationTime < 0 )
					animationTime += totalTime;
								
				currentFrame = frames->getFrame( (int)(animationTime/timePerFrame ) );
			}
			
			inline void advance( float dt )
			{				
				setAnimationTime( animationTime + dt );				
			}
			
		protected:			
			
			Texture* currentFrame;
			
			float animationTime, totalTime, timePerFrame, mElapsedLoops;
		};
		
	public:
		
		Vector pixelScale;
		bool pixelPerfect;
		
		AnimatedQuad( GameState* level, const Vector& pos, bool pixelPerfect = true );
		
		virtual ~AnimatedQuad()
		{
			//HACK MEMLEAK
			//if( animation ) 
			//	delete animation;
		}
		
		virtual void reset();
		
		///forces an animation with the given frameSet
		inline void immediateAnimation( FrameSet* s, float timePerFrame )
		{
			DEBUG_ASSERT( s );
			
			animation->setup( s, timePerFrame );
			
			_setTexture( animation->getCurrentFrame() );
			
			_updateScreenSize();
		}
		
		void immediateAnimation( const String& name, float timePerFrame );
		
		///returns the default screen size for the current animation frame
		inline const Vector& getScreenSize()				
		{	
			return screenSize;		
		}
		
		inline FrameSet* getFrameSet()
		{
			DEBUG_ASSERT( animation );
			
			return animation->frames;
		}
		
		
		inline int getCurrentFrameNumber()
		{
			return animation->getCurrentFrameNumber();
		}

		inline float getCurrentAnimationLength()
		{
			return animation->getTotalTime();
		}

		inline int getAnimationElapsedLoops()
		{
			return animation->getElapsedLoops();
		}
		
		inline void setAnimationTime( float t )
		{
			DEBUG_ASSERT( t >= 0 );
			DEBUG_ASSERT( animation );
			
			animation->setAnimationTime( t );
			
			_setTexture( animation->getCurrentFrame() );
		}
		
		inline void setAnimationPercent( float t )
		{
			DEBUG_ASSERT( animation );
			
			setAnimationTime( t * animation->getTotalTime() );
		}
						
		inline void advanceAnim( float dt )		
		{				
			DEBUG_ASSERT( animation );
					
			//active animation?
			if( animationSpeedMultiplier > 0 && animation->getTimePerFrame() > 0 )		
			{			
				DEBUG_ASSERT( animation->frames ); 
				DEBUG_ASSERT( animation->frames->getFrameNumber() );
				
				//update the renderState using the animation
				animation->advance( dt * animationSpeedMultiplier );				
				
				_setTexture( animation->getCurrentFrame() );
			}
		}
		
		inline void setFrame( uint i )
		{
			DEBUG_ASSERT( animation );
			DEBUG_ASSERT( animation->frames );
			
			animation->setFrame( i );
			
			_setTexture( animation->getCurrentFrame() );
		}
		
		inline void setAnimationSpeedMultiplier( float m )
		{
			DEBUG_ASSERT( m >= 0 );
			
			animationSpeedMultiplier = m;
		}
		
		virtual void onAction( float dt );
		
		virtual bool prepare( const Vector& viewportPixelRatio );
		
		void _updateScreenSize();
		
	protected:
		
		float animationSpeedMultiplier;		
		float animationTime;
		
		//animated quads are tied to a precise screen size
		Vector screenSize;
		
		//assigned animation
		Animation* animation;
				
		inline void _setTexture( Texture* t )
		{			
			setTexture( t, 0 );

			mesh = t->getOptimalBillboard();
		}		
	};
}

#endif