#pragma once

#include "dojo_common_header.h"

#include "Platform.h"
#include "Vector.h"
#include "Timer.h"
#include "InputSystem.h"
#include "Keyboard.h"
#include "SPSCQueue.h"

namespace Dojo {
	class XInputController;
	class Keyboard;

	class Win32Platform : public Platform {
	public:

		Win32Platform(const Table& config);
		virtual ~Win32Platform();

		virtual void initialize(Unique<Game> g);
		virtual void shutdown();

		void prepareThreadContext();

		virtual void setFullscreen(bool fullscreen);

		virtual bool isNPOTEnabled() {
			return true; //it always is on windows
		}

		virtual void acquireContext();
		void submitFrame() override;

		virtual void step(float dt);
		virtual void loop();

		virtual PixelFormat loadImageFile(std::vector<uint8_t>& imageData, utf::string_view path, uint32_t& width, uint32_t& height, int& pixelSize);

		virtual utf::string_view getAppDataPath() override;
		virtual utf::string_view getResourcesPath() override;
		virtual utf::string_view getRootPath() override;
		virtual utf::string_view getPicturesPath() override;
		virtual utf::string_view getShaderCachePath() override;

		virtual void openWebPage(utf::string_view site);

		virtual void setMouseLocked(bool locked) override;

		void mouseWheelMoved(int wheelZ);

		void mousePressed(int cursorX, int cursorY, Touch::Type type);
		void mouseMoved(int cursorX, int cursorY);
		void mouseReleased(int cursorX, int cursorY, Touch::Type type);

		void keyPressed(int kc);
		void keyReleased(int kc);

		float getFrameInterval() {
			return frameInterval;
		}

		void setVSync(int interval = 1);

	private:
		Timer frameTimer;

		Vector cursorPos, prevCursorPos;

		bool dragging;
		bool mMousePressed;

		bool _initializeWindow(utf::string_view caption, int w, int h);

	private:

		int lastPressedText;

		float frameInterval;
		int mFramesToAdvance;
		bool mouseLocked = false;
		bool realMouseEvent = true;

		KeyCode mKeyMap[ 256 ];

		Keyboard mKeyboard;
		int clientAreaYOffset;

		Unique<XInputController> mXInputJoystick[ 4 ];

		Timer mStepTimer;

		utf::string mAppDataPath, mRootPath, mShaderCachePath;

		void _initKeyMap();

		void _adjustWindow();

		void _setFullscreen(bool f);

		void _pollDevices(float dt);
	};
}
