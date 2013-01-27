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
	public:
		
		///Animation wraps a single FrameSet with timing information. 
		/**
			\remark using directly an animation shouldn't be needed in most cases
		*/
		class Animation
		{
		public:		
			
			FrameSet* frames;
			
			///Creates a new animation with frames from FrameSet, advancing frame each timePerFrame seconds
			Animation( FrameSet* set, float timePerFrame ) :
			currentFrame( NULL ),
			mElapsedLoops(0)
			{
				setup( set, timePerFrame );
			}			
						
			///Sets up an existing animation with frames from FrameSet, advancing frame each timePerFrame seconds
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

			inline void _unset()
			{
				mElapsedLoops = 0;
				animationTime = 0;
			}
			
			///gets the currently bound texture
			inline Texture* getCurrentFrame()
			{					
				return currentFrame;
			}			
			
			///gets the current texture ID in the FrameSet
			inline int getCurrentFrameNumber()
			{
				return frames->getFrameIndex( currentFrame );
			}
			
			///gets the duration of each frame in seconds
			inline float getTimePerFrame()
			{
				return timePerFrame;
			}

			///gets the total seconds duration of an animation loop
			inline float getTotalTime()
			{
				return totalTime;
			}

			///gets the current animation time
			/** 
			\remark goes back to 0 when the animation loops! */
			inline float getCurrentTime()
			{
				return animationTime;
			}

			///returns how many loops have elapsed since the last setup
			inline int getElapsedLoops()
			{
				return mElapsedLoops;
			}
			
			///forces the animation to display a frame
			inline void setFrame( uint i )
			{
				DEBUG_ASSERT( frames );
				DEBUG_ASSERT( frames->getFrameNumber() > i );
				
				currentFrame = frames->getFrame( i );
				
				animationTime = i * timePerFrame;
			}
			
			///forces the animation to a given time
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
			
			///advances the animation of dt seconds; usually this needs to be called each frame
			inline void advance( float dt )
			{				
				setAnimationTime( animationTime + dt );				
			}
			
		protected:			
			
			Texture* currentFrame;
			
			float animationTime, totalTime, timePerFrame;
			int mElapsedLoops;
		};
		
		Vector pixelScale;
		bool pixelPerfect;
		
		///creates a new AnimatedQuad
		/**
			\param level the level it will be created in
			\param pos its position
			\param pixelPerfect if pixelPerfect, an objects' scale is bound to the pixel size of the current frame and to the pixel size of the current Viewport. A pixelPerfect object can still be scaled using pixelScale.*/
		AnimatedQuad( GameState* level, const Vector& pos, bool pixelPerfect = true );
		
		virtual ~AnimatedQuad()
		{
			//HACK MEMLEAK
			//if( animation ) 
			//	delete animation;
		}
		
		///resets the AnimatedQuad to a "post-constructor" state
		virtual void reset();
		
		///forces an animation with the given frameSet
		inline void immediateAnimation( FrameSet* s, float timePerFrame )
		{
			DEBUG_ASSERT( s );
			
			animation->setup( s, timePerFrame );
			
			_setTexture( animation->getCurrentFrame() );
		}
		
		///forces an animation with the given FrameSet
		/**
		 \remark the FrameSet named "name" has to exist in the GameState used to create the AnimatedQuad*/
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

		///gets how many loops have been elapsed on the current animation
		inline int getAnimationElapsedLoops()
		{
			return animation->getElapsedLoops();
		}

		inline Animation* getAnimation()
		{
			return animation;
		}

		///forces the animation to a given time
		inline void setAnimationTime( float t )
		{
			DEBUG_ASSERT( t >= 0 );
			DEBUG_ASSERT( animation );
			
			animation->setAnimationTime( t );
			
			_setTexture( animation->getCurrentFrame() );
		}
		
		///forces the animation to a given time ratio
		/** 
		\param t a ratio value where 0 is animation start and 1 is animation end; a value outside [0..1] will make the animation loop. */
		inline void setAnimationPercent( float t )
		{
			DEBUG_ASSERT( animation );
			
			setAnimationTime( t * animation->getTotalTime() );
		}
						
		///advances the current animation and changes the current texture if the frame was changed
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
		
		///forces a to display the FrameSet frame i of the current Animation
		inline void setFrame( uint i )
		{
			DEBUG_ASSERT( animation );
			DEBUG_ASSERT( animation->frames );
			
			animation->setFrame( i );
			
			_setTexture( animation->getCurrentFrame() );
		}
		
		///sets the speed multiplier that is used by advanceFrame. m = 1 means normal speed, m = 2 double speed, ...
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

			_updateScreenSize();
		}		
	};
}

#endif