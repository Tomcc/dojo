/*
 *  Sprite.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "AnimatedQuad.h"

namespace Dojo {
	///A Sprite is a wrapper around AnimatedQuad providing management for multiple registered Animations
	class Sprite : public AnimatedQuad {

	public:

		typedef std::vector<std::unique_ptr<Animation>> AnimationList;

		///Creates a new Sprite in a GameState, at position Pos, with the given default Animation (which must exist!)
		/**
			\param parent the parent object or level
			\param pos its position
			\param defaultAnimName the name of animation 0
			\param timePerFrame the frame time of the animation
			\param pixelPerfect if pixelPerfect, an objects' scale is bound to the pixel size of the current frame and to the pixel size of the current Viewport. A pixelPerfect object can still be scaled using pixelScale.*/
		Sprite(Object& parent, RenderLayer::ID layer, const utf::string& shaderName, const utf::string& defaultAnimName = String::Empty, float timePerFrame = -1, bool pixelPerfect = true);

		virtual ~Sprite();

		virtual void reset() override;

		///registers the given animation and returns its handle
		/**
		If timePerFrame is not specified, the animation tries to use the frameset's preferred
		*/
		int registerAnimation(FrameSet* set, float timePerFrame = -1);

		///registers an animation using the FrameSet named base
		int registerAnimation(const utf::string& base, float timePerFrame = -1);

		///sets the animation at the given index
		void setAnimation(int i);

		///returns the current animation Index
		int getAnimationIndex() {
			return mAnimationIdx;
		}

	protected:
		AnimationList animations;

		int mAnimationIdx;
	};
}
