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

		virtual PixelFormat loadImageFile(std::vector<byte>& imageData, const utf::string& path, uint32_t& width, uint32_t& height, int& pixelSize);

		virtual const utf::string& getAppDataPath() override;
		virtual const utf::string& getResourcesPath() override;
		virtual const utf::string& getRootPath() override;
		virtual const utf::string& getPicturesPath() override;
		virtual const utf::string& getShaderCachePath() override;

		virtual void openWebPage(const utf::string& site);

		virtual void setMouseLocked(bool locked) override;

		void mouseWheelMoved(int wheelZ);

		void mousePressed(int cursorX, int cursorY, Touch::Type type);
		void mouseMoved(int cursorX, int cursorY);
		void mouseReleased(int cursorX, int cursorY, Touch::Type type);

		void keyPressed(int kc);
		void keyReleased(int kc);

		void _callbackThread(float frameLength);

		float getFrameInterval() {
			return frameInterval;
		}

		void setVSync(int interval = 1);

	protected:
		Timer frameTimer;

		Vector cursorPos, prevCursorPos;

		bool dragging;
		bool mMousePressed;

		Unique<SPSCQueue<std::function<void()>>> mContextRequestsQueue;

		bool _initializeWindow(const utf::string& caption, int w, int h);

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
