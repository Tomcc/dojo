#ifndef Win32Platform_h__
#define Win32Platform_h__

#include "dojo_common_header.h"

#include "Platform.h"
#include "Vector.h"
#include "Timer.h"
#include "InputSystem.h"
#include "Keyboard.h"

#include <Poco/Thread.h>
#include <Poco/Mutex.h>
#include <Poco/Semaphore.h>
#include <Poco/Timer.h>
#include <queue>

namespace Dojo
{
	class XInputJoystick;
	class Keyboard;

	class Win32Platform : public Platform
	{
	public:

		Win32Platform( const Table& config );
		virtual ~Win32Platform();

		virtual void initialise();
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
		virtual void loop( float frameTime );

		virtual GLenum loadImageFile( void*& bufptr, const String& path, int& width, int& height, int & pixelSize );
		
		virtual String getAppDataPath();
		virtual String getRootPath();
		
		virtual void openWebPage( const String& site );

		void mouseMoved( int cursorX, int cursorY );
		void mouseWheelMoved( int wheelZ );
		void mousePressed( int cursorX, int cursorY, int id );
		void mouseReleased(	int cursorX, int cursorY, int id );

		void keyPressed( int kc );
		void keyReleased( int kc );	

		void _callbackThread( float frameLength );

		inline float getFrameInterval()
		{
			return frameInterval;
		}

		void setVSync(int interval=1)
		{
			typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC)( int );
			PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT;

			char *extensions = (char*)glGetString( GL_EXTENSIONS );

			if( strstr( extensions, "WGL_EXT_swap_control" ) == 0 )
				return; // Error: WGL_EXT_swap_control extension not supported on your computer.\n");
			else
			{
				wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress( "wglSwapIntervalEXT" );

				if( wglSwapIntervalEXT )
					wglSwapIntervalEXT(interval);
			}
		}

		///invoke function called by the frame timer
		void _invoke( Poco::Timer& timer );

	protected:

		HINSTANCE hInstance;    // window app instance
		HWND hwnd;      // handle for the window
		HDC   hdc;      // handle to device context
		HGLRC hglrc;    // handle to OpenGL rendering context

		MSG msg;

		int width, height;

		Timer frameTimer;

		Poco::Semaphore frameStart;

		Vector cursorPos, prevCursorPos;

		bool dragging;
		bool mMousePressed;

		//context sharing stuff needed for multithread creation
		typedef std::queue< HGLRC* > ContextRequestsQueue;
		ContextRequestsQueue mContextRequestsQueue;
		Poco::Mutex mCRQMutex;

		bool _initialiseWindow( const String& caption, uint w, uint h );
		
	private:

		uint lastPressedText;

		float frameInterval;
		int mFramesToAdvance;

		FT_Library freeType;

		KeyCode mKeyMap[ 256 ];

		Keyboard mKeyboard;

		XInputJoystick* mXInputJoystick[ 4 ];

		void _initKeyMap();

		void _adjustWindow();

		void _setFullscreen( bool f );

		void _pollDevices( float dt );
	};
}

#endif // Win32Platform_h__