#ifndef Win32Platform_h__
#define Win32Platform_h__

#include "dojo_common_header.h"

#include <OIS.h>

#include "Platform.h"

#include "Timer.h"

namespace Dojo
{
	class Win32Platform : public Platform, public OIS::MouseListener, public OIS::KeyListener
	{
	public:

		Win32Platform();

		virtual void initialise();
		virtual void shutdown();

		virtual void acquireContext();
		virtual void present();

		virtual void step( float dt );
		virtual void loop( float frameTime );

		virtual std::string getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path );
		virtual void getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out );
		virtual uint loadFileContent( char*& bufptr, const std::string& path );
		virtual void loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height, bool POT );

		virtual bool mouseMoved( const OIS::MouseEvent& arg );
		virtual bool mousePressed( const OIS::MouseEvent& arg, OIS::MouseButtonID id );
		virtual bool mouseReleased(	const OIS::MouseEvent& arg, OIS::MouseButtonID id );

		virtual bool keyPressed(const OIS::KeyEvent &arg);
		virtual bool keyReleased(const OIS::KeyEvent &arg);	

	protected:

		HINSTANCE hInstance;    // window app instance
		HWND hwnd;      // handle for the window
		HDC   hdc;      // handle to device context
		HGLRC hglrc;    // handle to OpenGL rendering context

		MSG msg;

		int width, height;

		Timer frameTimer;

		OIS::InputManager* inputManager;
		OIS::Mouse* mouse;
		OIS::Keyboard* keys;

		bool dragging;

		bool _hasExtension( const std::string& type, const std::string& nameOrPath );
		std::string _toNormalPath( const std::string& path );

		bool _initialiseWindow( const std::string& caption, uint w, uint h );

	private:
	};
}

#endif // Win32Platform_h__