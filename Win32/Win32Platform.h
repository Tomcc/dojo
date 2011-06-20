#ifndef Win32Platform_h__
#define Win32Platform_h__

#include "dojo/dojo_common_header.h"

#include <Poco/Thread.h>
#include <Poco/Mutex.h>
#include <Poco/Semaphore.h>
#include <OIS/OIS.h>

#include "dojo/Platform.h"
#include "dojo/Vector.h"

#include "dojo/Timer.h"

namespace Dojo
{
	class Win32Platform : public Platform, public OIS::MouseListener, public OIS::KeyListener
	{
	public:

		Win32Platform();
		virtual ~Win32Platform();

		virtual void initialise();
		virtual void shutdown();

		virtual void acquireContext();
		virtual void present();

		virtual void step( float dt );
		virtual void loop( float frameTime );

		virtual String getCompleteFilePath( const String& name, const String& type, const String& path );
		virtual void getFilePathsForType( const String& type, const String& path, std::vector<String>& out );
		virtual uint loadFileContent( char*& bufptr, const String& path );
		virtual void loadPNGContent( void*& bufptr, const String& path, uint& width, uint& height );
		virtual uint loadAudioFileContent( ALuint& buffer, const String& path );
		
		virtual void load(  Table* dest, const String& relPath = String::EMPTY );
		virtual void save( Table* table, const String& relPath = String::EMPTY );

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

		bool _hasExtension( const String& type, const String& nameOrPath );
		String _toNormalPath( const String& path );

		String _getUserDirectory();
		String _getFilename( Table* dest, const String& absPath );

		bool _initialiseWindow( const String& caption, uint w, uint h );
		void _initialiseOIS();

	private:

		//hack needed because of OIS's bug
		uint lastPressedText;

		float frameInterval;

		FT_Library freeType;

		HWND _createDummyAAWindow();
		void _destroyDummyAAWindow( HWND w );

		int _getBestMSAAPF();
		int _getBestAntiAliasingPF();
	};
}

#endif // Win32Platform_h__