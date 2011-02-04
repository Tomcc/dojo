#include "stdafx.h"

#include "Win32Platform.h"

#include <Poco/DirectoryIterator.h>

#include <Freeimage.h>

#include <OIS.h>

#include "Render.h"
#include "Game.h"

using namespace Dojo;

LRESULT CALLBACK WndProc(   HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam ) 
{
	switch( message )
	{
	case WM_CREATE:
		Beep( 50, 10 );
		return 0;
		break;

	case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT ps;
			hdc = BeginPaint( hwnd, &ps );
			// don't draw here.  draw in the draw() function.
			EndPaint( hwnd, &ps );
		}
		return 0;
		break;

	case WM_KEYDOWN:
		switch( wparam )
		{
		case VK_ESCAPE:
			PostQuitMessage( 0 );
			break;
		default:
			break;
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage( 0 ) ;
		return 0;
		break;
	}

	return DefWindowProc( hwnd, message, wparam, lparam );
}




Win32Platform::Win32Platform() :
Platform()
{

}

void Win32Platform::_initialiseWindow()
{
	std::string windowCaption = "lolcopter";

	HWND consoleWindow = GetConsoleWindow();
	hInstance = (HINSTANCE)GetWindowLong( consoleWindow, GWL_HINSTANCE );

	WNDCLASS wc;
	wc.cbClsExtra = 0; 
	wc.cbWndExtra = 0; 
	wc.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );         
	wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );     
	wc.hInstance = hInstance;         
	wc.lpfnWndProc = WndProc;         
	wc.lpszClassName = TEXT( "DojoOpenGLWindow" );
	wc.lpszMenuName = 0; 
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

	// Register that class with the Windows O/S..
	RegisterClass(&wc);

	RECT rect;
	SetRect( &rect, 50,  // left
		50,  // top
		850, // right
		650 ); // bottom

	// Save width and height off.
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	AdjustWindowRect( &rect, WS_OVERLAPPEDWINDOW, false);

	// AdjustWindowRect() expands the RECT
	// so that the CLIENT AREA (drawable region)
	// has EXACTLY the dimensions we specify
	// in the incoming RECT.

	///////////////////
	// NOW we call CreateWindow, using
	// that adjusted RECT structure to
	// specify the width and height of the window.
	hwnd = CreateWindowA("DojoOpenGLWindow",
		(LPCSTR)windowCaption.c_str(),
		WS_OVERLAPPEDWINDOW,
		rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		NULL, NULL,
		hInstance, NULL);

	// check to see that the window was created successfully!
	if( hwnd == NULL )
		FatalAppExit( NULL, TEXT("CreateWindow() failed!") );

	// and show.
	ShowWindow( hwnd, SW_SHOWNORMAL );

	hdc = GetDC( hwnd );
	// CREATE PFD:
	PIXELFORMATDESCRIPTOR pfd = { 0 };  // create the pfd,
	// and start it out with ALL ZEROs in ALL of its fields.

	pfd.nSize = sizeof( PIXELFORMATDESCRIPTOR );    // just its size
	pfd.nVersion = 1;   // always 1

	pfd.dwFlags = PFD_SUPPORT_OPENGL |  // OpenGL support - not DirectDraw
		PFD_DOUBLEBUFFER   |  // double buffering support
		PFD_DRAW_TO_WINDOW;   // draw to the app window, not to a bitmap image

	pfd.iPixelType = PFD_TYPE_RGBA ;    // red, green, blue, alpha for each pixel
	pfd.cColorBits = 24;                // 24 bit == 8 bits for red, 8 for green, 8 for blue.
	// This count of color bits EXCLUDES alpha.

	pfd.cDepthBits = 32;                // 32 bits to measure pixel depth.  That's accurate!

	int chosenPixelFormat = ChoosePixelFormat( hdc, &pfd );

	if( chosenPixelFormat == 0 )
		return;

	if ( !SetPixelFormat( hdc, chosenPixelFormat, &pfd ) )
		return;

	hglrc = wglCreateContext( hdc );
	wglMakeCurrent( hdc, hglrc );
}

void Win32Platform::initialise()
{
	_initialiseWindow();

	render = new Render( this, width, height, 1 );
}

void Win32Platform::shutdown()
{
	// and a cheesy fade exit
	AnimateWindow( hwnd, 200, AW_HIDE | AW_BLEND );
}

void Win32Platform::acquireContext()
{

}

void Win32Platform::present()
{
	SwapBuffers( hdc );
}

void Win32Platform::loop()
{
	if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
	{
		if( msg.message == WM_QUIT )
		{
			return;
		}

		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	game->onLoop( Game::UPDATE_INTERVAL_CAP );

	render->render();
	//sound->update( Game::UPDATE_INTERVAL_CAP );
}

std::string Win32Platform::getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path )
{
	//semplicemente il path relativo all'exe
	return path + "/" + name + "." + type; 
}

bool Win32Platform::_hasExtension( const std::string& ext, const std::string& nameOrPath )
{
	return nameOrPath.size() > ext.size() && ext == nameOrPath.substr( nameOrPath.size() - ext.size() );
}

void Win32Platform::getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out )
{
	Poco::DirectoryIterator itr( path );

	std::string extension = "." + type;

	while( itr.name().size() )
	{
		const std::string& name = itr.name();

		if( _hasExtension( extension, name ) )
			out.push_back( itr->path() );

		++itr;
	}
}

uint Win32Platform::loadFileContent( char*& bufptr, const std::string& path )
{
	using namespace std;
	
	fstream file( path.c_str(), ios_base::in | ios_base::ate );

	if( !file.is_open() )
		return 0;

	uint size = file.tellg();

	bufptr = (char*)malloc( size );

	file.read( bufptr, size );

	file.close();

	return size;
}

void Win32Platform::loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height )
{
	//puo' caricare tutto ma per coerenza meglio limitarsi alle PNG (TODO: usare freeimage su iPhone?)
	if( !_hasExtension( ".png", path ) )
		return;

	//image format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	//pointer to the image, once loaded
	FIBITMAP *dib = NULL;

	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(path.c_str(), 0);
	//if still unknown, try to guess the file format from the file extension
	if(fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(path.c_str());
	//if still unkown, return failure
	if(fif == FIF_UNKNOWN)
		return;

	//check that the plugin has reading capabilities and load the file
	if(FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, path.c_str());
	//if the image failed to load, return failure
	if(!dib)
		return;

	//retrieve the image data
	bufptr = (void*)FreeImage_GetBits(dib);
	//get the image width and height, and size per pixel
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
}