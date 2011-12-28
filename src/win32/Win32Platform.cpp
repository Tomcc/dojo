#include "stdafx.h"

#include "win32/Win32Platform.h"
#include "win32/WGL_ARB_multisample.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ShellAPI.h>
#include <ShlObj.h>

#include <Freeimage.h>
#include <al/alut.h>
#include <gl/glu.h>

#include <gl/wglext.h>

#include <Poco/Path.h>

#include "Render.h"
#include "Game.h"
#include "Utils.h"
#include "Table.h"
#include "FontSystem.h"
#include "dojomath.h"
#include "SoundManager.h"
#include "InputSystem.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

using namespace Dojo;
using namespace OIS;

LRESULT CALLBACK WndProc(   HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam ) 
{
	switch( message )
	{
	case WM_CREATE:
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

Win32Platform::Win32Platform( const Table& config ) :
Platform( config ),
dragging( false ),
cursorPos( 0,0 ),
frameStart( 1 ),
frameInterval(0)
{
	/*
#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF |_CRTDBG_LEAK_CHECK_DF );
#endif
	*/

	frameStart.wait();
}

Win32Platform::~Win32Platform()
{

}

bool Win32Platform::_initialiseWindow( const String& windowCaption, uint w, uint h )
{
	hInstance = (HINSTANCE)GetModuleHandle(NULL);

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
	rect.top = 10;
	rect.left = 100;
	rect.bottom = rect.top + h + 7;
	rect.right = rect.left + w;

	width = w;
	height = h;

	DWORD dwstyle = WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU;
	
	AdjustWindowRect( &rect, dwstyle, true);

	// AdjustWindowRect() expands the RECT
	// so that the CLIENT AREA (drawable region)
	// has EXACTLY the dimensions we specify
	// in the incoming RECT.

	///////////////////
	// NOW we call CreateWindow, using
	// that adjusted RECT structure to
	// specify the width and height of the window.

	hwnd = CreateWindowA("DojoOpenGLWindow",
		windowCaption.ASCII().c_str(),
		dwstyle,  //non-resizabile
		rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		NULL, NULL,
		hInstance, NULL);

	if( hwnd == NULL )
		return false;

	// and show.
	ShowWindow( hwnd, SW_SHOWNORMAL );

	hdc = GetDC( hwnd );
	// CREATE PFD:
	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER |							// Must Support Double Buffering
		PFD_SWAP_EXCHANGE,
		PFD_TYPE_RGBA,								// Request An RGBA Format
		32, 										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	int chosenPixelFormat;
	ChooseAntiAliasingPixelFormat( chosenPixelFormat, config.getInt( "MSAA" ));

	if( chosenPixelFormat == 0 )
		return false;

	if ( !SetPixelFormat( hdc, chosenPixelFormat, &pfd ) )
		return false;

	hglrc = wglCreateContext( hdc );

	return wglMakeCurrent( hdc, hglrc )>0;
}

void Win32Platform::_initialiseOIS()
{
	OIS::ParamList params;

	//convert the hwnd to std strng
	std::stringstream tostring;
	tostring << (uint)hwnd;
	params.insert( std::make_pair( "WINDOW", tostring.str() ) );

	//avoid cursor grab
	params.insert( std::make_pair( "w32_mouse", "DISCL_FOREGROUND" ) );
	params.insert( std::make_pair( "w32_mouse", "DISCL_NONEXCLUSIVE" ) );

	inputManager = InputManager::createInputSystem( params );

	if( inputManager->getNumberOfDevices( OISKeyboard ) > 0)
	{
		keys = (Keyboard*)inputManager->createInputObject( OISKeyboard, true );
		keys->setEventCallback( this );

		keys->setTextTranslation( OIS::Keyboard::Unicode );
	}

	if( inputManager->getNumberOfDevices( OISMouse ) > 0 )
	{
		mouse = (Mouse*)inputManager->createInputObject( OISMouse, true );
		mouse->setEventCallback( this );
		mouse->getMouseState().width = width;
		mouse->getMouseState().height = height;
	}

	setVSync( !config.getBool( "disable_vsync" ) );		
}

void Win32Platform::initialise()
{
	DEBUG_ASSERT( game );

	//create user dir if not existing
	String userDir = getAppDataPath() + '/' + game->getName();

	CreateDirectoryA( userDir.ASCII().c_str(), NULL );

	//just use the game's preferred settings
	if( !_initialiseWindow( game->getName(), game->getNativeWidth(), game->getNativeHeight() ) )
		return;

	glewInit();

	DEBUG_MESSAGE( "Creating OpenGL context...");
	DEBUG_MESSAGE ("querying GL info... ");
	DEBUG_MESSAGE ("vendor: " << glGetString (GL_VENDOR));
	DEBUG_MESSAGE ("renderer: " << glGetString (GL_RENDERER));
	DEBUG_MESSAGE ("version: " << glGetString (GL_VERSION));

	render = new Render( width, height, 1, Render::RO_LANDSCAPE_LEFT );

	sound = new SoundManager();

	//initialize OIS to emulate the touch
	_initialiseOIS();

	input = new InputSystem();
	fonts = new FontSystem();

	//start the game
	game->begin();
}


void Win32Platform::shutdown()
{
	//destroy game
	game->end();

	delete game;

	//destroy managers
	delete render;
	delete sound;
	delete input;
	delete fonts;

	//clean OIS
	inputManager->destroyInputObject( mouse );
	inputManager->destroyInputObject( keys );

	OIS::InputManager::destroyInputSystem( inputManager );

	// and a cheesy fade exit
	AnimateWindow( hwnd, 200, AW_HIDE | AW_BLEND );
}

void Win32Platform::acquireContext()
{
	wglMakeCurrent( hdc, hglrc );
}

void Win32Platform::present()
{
	SwapBuffers( hdc );
}

void Win32Platform::step( float dt )
{
	Timer timer;
	
	//cattura l'input prima del gameplay
	keys->capture();
	mouse->capture();

	game->loop( dt);

	render->render();
	
	sound->update( dt );
	
	realFrameTime = (float)timer.getElapsedTime();
}

void stepCallback( void* platform )
{
	Win32Platform* self = (Win32Platform*)platform;

	self->_callbackThread( self->getFrameInterval() );
}

void Win32Platform::_callbackThread( float frameLength )
{
	while( running )
	{
		try
		{
			frameStart.set();

		}
		catch( Poco::SystemException e )
		{

		}

		Poco::Thread::sleep( (long)(frameLength * 1000.f) );
	}
}

void Win32Platform::loop( float frameTime )
{
	frameInterval = frameTime;

	frameTimer.reset();

	//start timer thread
	Poco::Thread t;
	if( frameInterval )
		t.start( stepCallback, this );

	Timer timer;
	running = true;
	while( running )
	{
		if( frameInterval > 0 )
			frameStart.wait();


		while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if( msg.message == WM_QUIT )
				running = false;

			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		step( (float)timer.deltaTime() );
	}

	if( frameTime )
		t.join();
}

bool Win32Platform::mousePressed( const MouseEvent& arg, MouseButtonID id )
{
	dragging = true;

	cursorPos.x = (float)arg.state.X.abs;
	cursorPos.y = (float)arg.state.Y.abs;

	input->_fireTouchBeginEvent( cursorPos );

	return true;
}

bool Win32Platform::mouseMoved( const MouseEvent& arg )
{
	if( dragging )
	{
		cursorPos.x = (float)arg.state.X.abs;
		cursorPos.y = (float)arg.state.Y.abs;

		input->_fireTouchMoveEvent( cursorPos );
	}
	return true;
}

bool Win32Platform::mouseReleased( const MouseEvent& arg, MouseButtonID id )
{
	dragging = false;

	cursorPos.x = (float)arg.state.X.abs;
	cursorPos.y = (float)arg.state.Y.abs;

	input->_fireTouchEndEvent( cursorPos );

	return true;
}

bool Win32Platform::keyPressed(const OIS::KeyEvent &arg)
{
	lastPressedText = arg.text; 
	
	input->_fireKeyPressedEvent( arg.text, arg.key );

	return true;
}

bool Win32Platform::keyReleased(const OIS::KeyEvent &arg)
{
	input->_fireKeyReleasedEvent( lastPressedText, arg.key );

	return true;
}

bool Win32Platform::loadPNGContent( void*& bufptr, const String& path, int& width, int& height )
{
	//puo' caricare tutto ma per coerenza meglio limitarsi alle PNG (TODO: usare freeimage su iPhone?)
	if( !Utils::hasExtension( ".png", path ) )
		return false;

	void* data;

	//image format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	//pointer to the image, once loaded
	FIBITMAP *dib = NULL;

	std::string ansipath = path.ASCII();

	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(ansipath.c_str(), 0);
	//if still unknown, try to guess the file format from the file extension
	if(fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(ansipath.c_str());
	//if still unkown, return failure
	if(fif == FIF_UNKNOWN)
		return false;

	//check that the plugin has reading capabilities and load the file
	if(FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, ansipath.c_str());
	//if the image failed to load, return failure
	if(!dib)
		return false;

	//retrieve the image data
	data = (void*)FreeImage_GetBits(dib);
	//get the image width and height, and size per pixel
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	
	uint pixelSize = FreeImage_GetBPP(dib)/8;
	
	uint size = width*height*4;
	bufptr = malloc( size );
	
	//swap R and B and invert image while copying
	byte* in, *out;
	for( int i = 0, ii = height-1; i < height ; ++i, --ii )
	{
		for( int j = 0; j < width; ++j )
		{
			out = (byte*)bufptr + (j + i*width)*4;
			in = (byte*)data + (j + ii*width)*pixelSize;

			out[3] = (pixelSize > 3) ? in[3] : 0;
 			out[2] = in[0];
			out[1] = in[1];
			out[0] = in[2];
		}
	}
	
	FreeImage_Unload( dib );

	return true;
}

String Win32Platform::getAppDataPath()
{
	char szPath[MAX_PATH];

	SHGetFolderPathA(
		hwnd, 
		CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE, 
		NULL, 
		0, 
		szPath);

	String dir( szPath );

	Utils::makeCanonicalPath( dir );

	return dir; 
}

String Win32Platform::getRootPath()
{
	return String( Poco::Path::current() );
}

void Win32Platform::openWebPage( const String& site )
{
	ShellExecuteA(hwnd, "open", site.ASCII().c_str(), NULL, NULL, SW_SHOWNORMAL);
}