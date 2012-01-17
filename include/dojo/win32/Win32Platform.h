#ifndef Win32Platform_h__
#define Win32Platform_h__

#include "dojo_common_header.h"

#include "Platform.h"
#include "Vector.h"
#include "Timer.h"

#include <Poco/Thread.h>
#include <Poco/Mutex.h>
#include <Poco/Semaphore.h>
#include <OIS/OIS.h>
#include <queue>

namespace Dojo
{
	class Win32Platform : public Platform, public OIS::MouseListener, public OIS::KeyListener
	{
	public:

		Win32Platform( const Table& config );
		virtual ~Win32Platform();

		virtual void initialise();
		virtual void shutdown();

		void prepareThreadContext();

		virtual bool isNPOTEnabled()
		{
			return true; //it always is on windows
		}

		virtual void acquireContext();
		virtual void present();

		virtual void step( float dt );
		virtual void loop( float frameTime );

		virtual bool loadPNGContent( void*& bufptr, const String& path, int& width, int& height );
		
		virtual String getAppDataPath();
		virtual String getRootPath();

		virtual void openWebPage( const String& site );

		virtual bool mouseMoved( const OIS::MouseEvent& arg );
		virtual bool mousePressed( const OIS::MouseEvent& arg, OIS::MouseButtonID id );
		virtual bool mouseReleased(	const OIS::MouseEvent& arg, OIS::MouseButtonID id );

		virtual bool keyPressed(const OIS::KeyEvent &arg);
		virtual bool keyReleased(const OIS::KeyEvent &arg);	

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

	protected:

		HINSTANCE hInstance;    // window app instance
		HWND hwnd;      // handle for the window
		HDC   hdc;      // handle to device context
		HGLRC hglrc;    // handle to OpenGL rendering context

		MSG msg;

		int width, height;

		Timer frameTimer;

		Poco::Semaphore frameStart;

		OIS::InputManager* inputManager;
		OIS::Mouse* mouse;
		OIS::Keyboard* keys;

		Vector cursorPos;

		bool dragging;

		//context sharing stuff needed for multithread creation
		typedef std::queue< HGLRC* > ContextRequestsQueue;
		ContextRequestsQueue mContextRequestsQueue;
		Poco::Mutex mCRQMutex;

		bool _initialiseWindow( const String& caption, uint w, uint h );
		void _initialiseOIS();
		
	private:

		//hack needed because of OIS's bug
		uint lastPressedText;

		float frameInterval;

		FT_Library freeType;
	};
}

#endif // Win32Platform_h__