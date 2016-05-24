/*
 *  Game.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once

#include "dojo_common_header.h"

#include "StateInterface.h"
#include "ApplicationListener.h"

namespace Dojo {
	///Game is the main game-code class in Dojo.
	/**
	It inherits the StateInterface class making it the first event listener, which listens directly to the Platform-sent events.

	Implementing any game begins with implementing Game's onBegin(), onLoop() and onEnd() with custom code:
	-onBegin() is called when the Platform has been initialized and the game is ready to start
	-onLoop( dt ) is called each frame after the input has been gathered and before the graphics are rendered
	-onEnd() is called when the application is closed
	*/
	class Game : public StateInterface, public ApplicationListener {
	public:

		///Creates a new Game
		/**
		Game's parameters will be used by the Platform to initialize the display area and the Run Loop
		\param name the name used on the window caption, program description, Userdata and Documents directory
		\param nativeWidth the width that the game desires to run at. 0 means fullscreen, any resolution
		\param nativeHeight the height that the game desires to run at. 0 means fullscreen, any resolution
		\param nativeOrientation the orientation on devices which support display rotation
		\param nativeFrequency the update rate of this game, or how much should a frame last. Default is 1/60 s or the VSync duration if enabled
		\param maxFrameLength the max frame dt that will be propagated to the game, to avoid integration glitch on long frames
		\param requireDepthBuffer set to false if the game won't use any perspective/depth sorted layer to use less VRAM
		*/
		Game(const utf::string& name, int nativeWidth = 0, int nativeHeight = 0, Orientation nativeOrientation = DO_LANDSCAPE_RIGHT, float nativeFrequency = 1.f / 60.f, float maxFrameLength = 1.f / 30.f, bool requireDepthBuffer = true);

		virtual ~Game();

		const utf::string& getName() const {
			return name;
		}

		uint32_t getNativeWidth() const {
			return nativeWidth;
		}

		uint32_t getNativeHeight()  const {
			return nativeHeight;
		}

		float getNativeFrameLength() const {
			return mNativeFrameLength;
		}

		float getMaximumFrameLength() const {
			return mMaxFrameLength;
		}

		Orientation getNativeOrientation() const {
			return nativeOrientation;
		}

		bool getRequiresDepthBuffer() const {
			return mRequireDepthBuffer;
		}

		virtual void onWindowResolutionChanged(int w, int h) {

		}

		
		bool isRunning() const {
			return running;
		}

		void stop() {
			running = false;
		}

	protected:

		uint32_t nativeWidth, nativeHeight;
		float mNativeFrameLength, mMaxFrameLength;
		bool mRequireDepthBuffer;

		Orientation nativeOrientation;

		utf::string name;

		bool running = true;
	};
}
