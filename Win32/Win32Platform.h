#ifndef Win32Platform_h__
#define Win32Platform_h__

#include "dojo_common_header.h"

#include "Platform.h"

namespace Dojo
{
	class Win32Platform : public Platform
	{
	public:

		Win32Platform();

		virtual void initialise();
		virtual void shutdown();

		virtual void acquireContext();
		virtual void present();

		virtual void loop();

		virtual std::string getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path );
		virtual void getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out );
		virtual uint loadFileContent( char*& bufptr, const std::string& path );
		virtual void loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height, bool POT );

	protected:

		HINSTANCE hInstance;    // window app instance
		HWND hwnd;      // handle for the window
		HDC   hdc;      // handle to device context
		HGLRC hglrc;    // handle to OpenGL rendering context

		MSG msg;

		int width, height;      // the desired width and

		bool _hasExtension( const std::string& type, const std::string& nameOrPath );
		std::string _toNormalPath( const std::string& path );

		bool _initialiseWindow( const std::string& caption, uint w, uint h );

	private:
	};
}

#endif // Win32Platform_h__