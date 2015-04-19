/*
 *  AnimatedQuad.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/13/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "Renderable.h"

namespace Dojo {
	class FrameSet;

	class AnimatedQuad : public Renderable {
	public:

		///Animation wraps a single FrameSet with timing information. 
		/**
			\remark using directly an animation shouldn't be needed in most cases
		*/
		class Animation {
		public:

			FrameSet* frames;

			///Creates a new animation with frames from FrameSet, advancing frame each timePerFrame seconds
			Animation(FrameSet* set, float timePerFrame);

			///Sets up an existing animation with frames from FrameSet, advancing frame each timePerFrame seconds
			void setup(FrameSet* set, float tpf);

			void _unset() {
				mElapsedLoops = 0;
				animationTime = 0;
			}

			///gets the currently bound texture
			Texture* getCurrentFrame() {
				return currentFrame;
			}

			///gets the current texture ID in the FrameSet
			int getCurrentFrameNumber();

			///gets the duration of each frame in seconds
			float getTimePerFrame() {
				return timePerFrame;
			}

			///gets the total seconds duration of an animation loop
			float getTotalTime() {
				return totalTime;
			}

			///gets the current animation time
			/** 
			\remark goes back to 0 when the animation loops! */
			float getCurrentTime() {
				return animationTime;
			}

			///returns how many loops have elapsed since the last setup
			int getElapsedLoops() {
				return mElapsedLoops;
			}

			///forces the animation to display a frame
			/**
			if i is out of bounds, it will be wrapped on the existing frames.
			*/
			void setFrame(int i);

			///forces the animation to a given time
			void setAnimationTime(float t);

			///advances the animation of dt seconds; usually this needs to be called each frame
			void advance(float dt);

		protected:

			Texture* currentFrame;

			float animationTime, totalTime, timePerFrame;
			int mElapsedLoops;
		};

		Vector pixelScale;
		bool pixelPerfect;

		///creates a new AnimatedQuad
		/**
			\param level the parent it will be created for
			\param pos its position
			\param immediateAnim the name of the animation it will be set to
			\param timePerFrame immediate animation's time per frame */
		AnimatedQuad(Object* parent, const Vector& pos, const String& immediateAnim = String::EMPTY, float timePerFrame = 0.0f);

		virtual ~AnimatedQuad() {
			//HACK MEMLEAK
			//if( animation ) 
			//	delete animation;
		}

		///resets the AnimatedQuad to a "post-constructor" state
		virtual void reset();

		///forces an animation with the given frameSet
		void immediateAnimation(FrameSet* s, float timePerFrame);

		///forces an animation with the given FrameSet
		/**
		 \remark the FrameSet named "name" has to exist in the GameState used to create the AnimatedQuad*/
		void immediateAnimation(const String& name, float timePerFrame);

		///returns the default screen size for the current animation frame
		const Vector& getScreenSize() {
			return screenSize;
		}

		FrameSet* getFrameSet() {
			DEBUG_ASSERT( animation != nullptr, "getFrameSet: no animation set" );

			return animation->frames;
		}


		int getCurrentFrameNumber() {
			return animation->getCurrentFrameNumber();
		}

		float getCurrentAnimationLength() {
			return animation->getTotalTime();
		}

		///gets how many loops have been elapsed on the current animation
		int getAnimationElapsedLoops() {
			return animation->getElapsedLoops();
		}

		Animation* getAnimation() {
			return animation;
		}

		///forces the animation to a given time
		void setAnimationTime(float t);

		///forces the animation to a given time ratio
		/** 
		\param t a ratio value where 0 is animation start and 1 is animation end; a value outside [0..1] will make the animation loop. */
		void setAnimationPercent(float t);

		///advances the current animation and changes the current texture if the frame was changed
		void advanceAnim(float dt);

		///forces a to display the FrameSet frame i of the current Animation
		void setFrame(int i);

		///sets the speed multiplier that is used by advanceFrame. m = 1 means normal speed, m = 2 double speed, ...
		void setAnimationSpeedMultiplier(float m);

		virtual void onAction(float dt);

		void _updateScreenSize();

	protected:

		float animationSpeedMultiplier;
		float animationTime;

		//animated quads are tied to a precise screen size
		Vector screenSize;

		//assigned animation
		Animation* animation;

		void _setTexture(Texture* t);
	};
}
