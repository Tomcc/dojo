#pragma once

#include "dojo_common_header.h"

#include "Platform.h"
#include "Vector.h"
#include "Timer.h"
#include "InputSystem.h"
#include "Keyboard.h"
#include "Pipe.h"

namespace Dojo
{
	class XInputController;
	class Keyboard;

	class Win32Platform : public Platform
	{
	public:

		Win32Platform( const Table& config );
		virtual ~Win32Platform();

		virtual void initialize( Game* g );
		virtual void shutdown();

		void prepareThreadContext();

		virtual void setFullscreen( bool fullscreen );

		virtual bool isNPOTEnabled()
		{
			return true; //it always is on windows
		}

		virtual void acquireContext();
		virtual void present();

		virtual void step( float dt );
		virtual void loop();

		virtual GLenum loadImageFile( void*& bufptr, const String& path, int& width, int& height, int & pixelSize );
		
		virtual const String& getAppDataPath();
		virtual const String& getResourcesPath();
		virtual const String& getRootPath();
		
		virtual void openWebPage( const String& site );

		virtual void setMouseLocked(bool locked) override;

		void mouseWheelMoved( int wheelZ );

		void mousePressed(int cursorX, int cursorY, Touch::Type type);
		void mouseMoved(int cursorX, int cursorY );
		void mouseReleased(int cursorX, int cursorY, Touch::Type type);

		void keyPressed( int kc );
		void keyReleased( int kc );	

		void _callbackThread( float frameLength );

		float getFrameInterval()
		{
			return frameInterval;
		}

		void setVSync(int interval=1);

	protected:

		HINSTANCE hInstance;    // window app instance
		HWND hwnd;      // handle for the window
		HDC   hdc;      // handle to device context
		HGLRC hglrc;    // handle to OpenGL rendering context

		MSG msg;

		int width, height;

		Timer frameTimer;

		Vector cursorPos, prevCursorPos;

		bool dragging;
		bool mMousePressed;

		//context sharing stuff needed for multithread creation
		struct ContextShareRequest
		{
			std::atomic<bool> done;
			ContextShareRequest() :
				done(false)
			{

			}

			HGLRC contextHandle;
		};

		typedef Pipe< ContextShareRequest* > ContextRequestsQueue;
		Unique<ContextRequestsQueue> mContextRequestsQueue;

		bool _initializeWindow( const String& caption, int w, int h );
		
	private:

		int lastPressedText;

		float frameInterval;
		int mFramesToAdvance;
		bool mouseLocked = false;
		bool realMouseEvent = true;

		FT_Library freeType;

		KeyCode mKeyMap[ 256 ];

		Keyboard mKeyboard;
		int clientAreaYOffset;

		Unique<XInputController> mXInputJoystick[ 4 ];

		Timer mStepTimer;

		String mAppDataPath, mRootPath;

		void _initKeyMap();

		void _adjustWindow();

		void _setFullscreen( bool f );

		void _pollDevices( float dt );
	};
}
