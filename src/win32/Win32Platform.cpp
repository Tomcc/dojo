#include "stdafx.h"

#include "win32/Win32Platform.h"
#include "win32/WGL_ARB_multisample.h"

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

#define WINDOWMODE_PROPERTIES (WS_BORDER | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS)

LRESULT CALLBACK WndProc(   HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam ) 
{
	Win32Platform* app = (Win32Platform*)Platform::getSingleton();

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

	case WM_DESTROY:
		PostQuitMessage( 0 ) ;
		return 0;

	//enter / exit unfocused state
	case WM_ACTIVATEAPP:
	case WM_ACTIVATE:
	case WM_SHOWWINDOW:	
		if( wparam == false ) //minimized or defocused
			app->_fireFocusLost();

		else 
			app->_fireFocusGained();

		return 0;

    case WM_MOUSEWHEEL: //mouse wheel moved
		app->mouseWheelMoved( (float)((short)HIWORD(wparam)) / (float)WHEEL_DELTA );
        return 0;

    case WM_LBUTTONDOWN:  //left down
	case WM_RBUTTONDOWN: //right up
	case WM_MBUTTONDOWN:

		app->mousePressed( LOWORD( lparam ), HIWORD( lparam ), WM_LBUTTONDOWN - message );
        return 0;

    case WM_LBUTTONUP:   //left up
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:

		app->mouseReleased( LOWORD( lparam ), HIWORD( lparam ), WM_LBUTTONUP - message );
        return 0;

    case WM_MOUSEMOVE:
		app->mouseMoved( LOWORD( lparam ), HIWORD( lparam ) );
        return 0;

    case WM_KEYDOWN:

		switch( wparam )
		{
#ifdef _DEBUG  //close with ESC automagically
		case VK_ESCAPE:
			PostQuitMessage( 0 );
			break;
#endif
		default:

			app->keyPressed( wparam );
			break;
		}
        return 0;

    case WM_SYSKEYDOWN:
		if( wparam == VK_F4 ) //listen for Alt+F4
		{
			PostQuitMessage(1);
			return 0;
		}
        break;

    case WM_KEYUP:

		app->keyReleased( wparam );
        return 0;

    case WM_CHAR:
        return 0;

    case WM_SIZE:

        return 0;

    case WM_ENTERSIZEMOVE: //this message is sent when the window is about to lose control in live resize
        //start a timer to keep getting updates at 30 fps
        //SetTimer( getWin32Window(), 1, 1./30., NULL );
        //_isInModalLoop = true;

        return 0;

    case WM_TIMER:
        //the window is currently in live resize, manually update the game!
        //if( pGame && pGame->isRunning() )
        //	pGame->update();

        return 0;

    case WM_EXITSIZEMOVE:
        //_isInModalLoop = false;
        //KillTimer( getWin32Window(), 1 );

        return 0;

    case WM_KILLFOCUS:
        //if we were fullscreen in OpenGL, we need to reset the original setup
        /*if( getVideoDriver() && DriverType == ox::video::EDT_OPENGL && getVideoDriver()->isFullscreen() )
        {
            bool success = _changeDisplayMode( false, ox::TDimension() );
            //hide window
            ShowWindow( hWnd, FALSE );

            DEBUG_ASSERT( success );
        }*/
        return 0;

    case WM_SETFOCUS:
        //if we are fullscreen in OpenGL, we need to change the res again
        /*if( getVideoDriver() && DriverType == ox::video::EDT_OPENGL && getVideoDriver()->isFullscreen() )
        {
            bool success = _changeDisplayMode( true, WindowSize );
            ShowWindow( hWnd, TRUE );
            DEBUG_ASSERT( success );
        }*/
        return 0;
	}

	return DefWindowProc( hwnd, message, wparam, lparam );
}

Win32Platform::Win32Platform( const Table& config ) :
Platform( config ),
dragging( false ),
mMousePressed( false ),
cursorPos( Vector::ZERO ),
frameStart( 1 ),
frameInterval(0)
{
	/*
#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF |_CRTDBG_LEAK_CHECK_DF );
#endif
	*/

	screenWidth = GetSystemMetrics( SM_CXSCREEN );
	screenHeight = GetSystemMetrics( SM_CYSCREEN );

	frameStart.wait();

	mFullscreen = false; //windows creates... windows by default
}

Win32Platform::~Win32Platform()
{
	setFullscreen( false ); //get out of fullscreen
}

void Win32Platform::_adjustWindow()
{
	RECT clientSize;
    //GetClientRect(HWnd, &clientSize);
    clientSize.top = 0;
    clientSize.left = 0;
    clientSize.right = game->getNativeWidth();
    clientSize.bottom = game->getNativeHeight();

    AdjustWindowRect(&clientSize, WINDOWMODE_PROPERTIES, FALSE);

    int realWidth = clientSize.right - clientSize.left;
    int realHeight = clientSize.bottom - clientSize.top;

    int windowLeft = (GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
    int windowTop = (GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;

    // Make any window in the background repaint themselves
    InvalidateRect( NULL, NULL, false );

    SetWindowPos(hwnd, HWND_NOTOPMOST, windowLeft, windowTop, realWidth, realHeight, 
        SWP_SHOWWINDOW);

    MoveWindow(hwnd, windowLeft, windowTop, realWidth, realHeight, TRUE);
}

bool Win32Platform::_initialiseWindow( const String& windowCaption, uint w, uint h )
{
	DEBUG_MESSAGE( "Creating " << w << "x" << h << " window" );

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
	rect.top = rect.left = 0;
	rect.bottom = rect.top + h + 7;
	rect.right = rect.left + w;

	width = w;
	height = h;

	DWORD dwstyle = WINDOWMODE_PROPERTIES;
	
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
		0,											// No Alpha Resource
		0,											// Shift Bit Ignored
		0,											// No Accumulation Resource
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Resource (Depth Resource)  
		0,											// No Stencil Resource
		0,											// No Auxiliary Resource
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
	bool err = wglMakeCurrent( hdc, hglrc )>0;
	glewInit();

// and show.
	ShowWindow( hwnd, SW_SHOWNORMAL );

	_adjustWindow();

	return err;
}

void Win32Platform::setFullscreen( bool fullscreen )
{
	if( fullscreen == mFullscreen )
		return;

	//set window style
	DWORD style = fullscreen ? (WS_POPUP | WS_VISIBLE) : WINDOWMODE_PROPERTIES;
	SetWindowLong(hwnd, GWL_STYLE, style);

	if( !fullscreen )
	{
		ChangeDisplaySettings( NULL, 0 );

		_adjustWindow(); //reset back
	}
	else
	{
		DEVMODE dm;
		memset(&dm, 0, sizeof(dm));
		dm.dmSize = sizeof(dm);
		dm.dmPelsWidth = game->getNativeWidth();
		dm.dmPelsHeight	= game->getNativeHeight();
		dm.dmBitsPerPel	= 32;
		dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		LONG ret = ChangeDisplaySettings( &dm, CDS_FULLSCREEN );

		DEBUG_ASSERT( ret == DISP_CHANGE_SUCCESSFUL );

		//WARNING MoveWindow can change backbuffer size
		MoveWindow(hwnd, 0, 0, dm.dmPelsWidth, dm.dmPelsHeight, TRUE);
	}

	ShowCursor( !fullscreen );

	mFullscreen = fullscreen;
}

void Win32Platform::initialise()
{
	DEBUG_ASSERT( game );

	DEBUG_MESSAGE( "Initializing Dojo Win32" );

	//create user dir if not existing
	String userDir = getAppDataPath() + '/' + game->getName();

	CreateDirectoryA( userDir.ASCII().c_str(), NULL );

	//just use the game's preferred settings
	if( !_initialiseWindow( game->getName(), game->getNativeWidth(), game->getNativeHeight() ) )
		return;

	setVSync( !config.getBool( "disable_vsync" ) );		
	
	render = new Render( width, height, DO_LANDSCAPE_LEFT );

	sound = new SoundManager();

	input = new InputSystem();
	fonts = new FontSystem();

	DEBUG_MESSAGE( "---- Game Launched!");

	//start the game
	game->begin();
}

void Win32Platform::prepareThreadContext()
{
	HGLRC context = 0;

	mCRQMutex.lock();

	mContextRequestsQueue.push( &context );
			
	mCRQMutex.unlock();

	//now busy-wait
#ifdef _DEBUG
	while( context == 0 );
#else
	//HACK
	Poco::Thread::sleep( 1000 );
#endif

	bool error = wglMakeCurrent( hdc, context )>0;

	glewInit();
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

///this function is called by Poco::Timer on the timer thread
void Win32Platform::_invoke( Poco::Timer& timer )
{
	try 
	{
		frameStart.set(); //enable the main loop to run again
	}
	catch(...)
	{

	}
}

void Win32Platform::step( float dt )
{
	Timer timer;
	
	//check if some other thread requested a new context
	mCRQMutex.lock();
	while( mContextRequestsQueue.size() )
	{
		HGLRC c = wglCreateContext(hdc); 
		
		bool success = wglShareLists(hglrc, c) != 0;

		DEBUG_ASSERT( success );

		*(mContextRequestsQueue.front()) = c; //assing back to the caller thread

		mContextRequestsQueue.pop();
	}
	mCRQMutex.unlock();

	game->loop( dt);

	render->render();
	
	sound->update( dt );
	
	realFrameTime = (float)timer.getElapsedTime();
}

void Win32Platform::loop( float frameTime )
{
	frameInterval = frameTime;

	frameTimer.reset();

	//start timer thread
	Poco::Timer frameCaller( 0, (long)(frameInterval * 1000.f) );
	Poco::TimerCallback< Win32Platform > callback( *this, &Win32Platform::_invoke );
	frameCaller.start( callback );

	Timer timer;
	running = true;
	while( running )
	{
		if( frameInterval > 0 )
			frameStart.wait();

		while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if( msg.message == WM_QUIT )
			{
				this->_fireTermination();
				running = false;
			}

			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		//never send a dt lower than the minimum!
		float dt = min( game->getMaximumFrameLength(), (float)timer.deltaTime() );

		step( dt );
	}
}

void Win32Platform::mousePressed( int cx, int cy, int id )
{
	//TODO use the button ID!
	mMousePressed = true;
	dragging = true;

	cursorPos.x = (float)cx;
	cursorPos.y = (float)cy;

	input->_fireTouchBeginEvent( cursorPos );
}

void Win32Platform::mouseWheelMoved( int wheelZ )
{
	cursorPos.z = (float)wheelZ;

	input->_fireScrollWheelEvent( cursorPos.z - prevCursorPos.z );

	prevCursorPos.z = cursorPos.z;
}

void Win32Platform::mouseMoved(  int cx, int cy  )
{
	cursorPos.x = (float)cx;
	cursorPos.y = (float)cy;

	if( dragging )	input->_fireTouchMoveEvent( cursorPos, prevCursorPos );
	else			input->_fireMouseMoveEvent( cursorPos, prevCursorPos );

	prevCursorPos = cursorPos;
}

void Win32Platform::mouseReleased( int cx, int cy, int id )
{
	//windows can actually send "released" messages whose "pressed" event was sent to another window
	//or used to awake the current one - send a fake mousePressed event if this happens!
	if( !mMousePressed  )
		mousePressed( cx, cy, id );

	mMousePressed  = false;
	dragging = false;

	cursorPos.x = (float)cx;
	cursorPos.y = (float)cy;

	input->_fireTouchEndEvent( cursorPos );
}

void Win32Platform::keyPressed( int kc )
{
	//TODO reimplement text!
	lastPressedText = 0; 

	input->_fireKeyPressedEvent( 0, (InputSystem::KeyCode)kc );
}

void Win32Platform::keyReleased( int kc )
{
	input->_fireKeyReleasedEvent( lastPressedText, (InputSystem::KeyCode)kc );
}

GLenum Win32Platform::loadImageFile( void*& bufptr, const String& path, int& width, int& height, int& pixelSize )
{
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
		return 0;

	//check that the plugin has reading capabilities and load the file
	if(FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, ansipath.c_str());
	//if the image failed to load, return failure
	if(!dib)
		return 0;

	//retrieve the image data
	data = (void*)FreeImage_GetBits(dib);
	//get the image width and height, and size per pixel
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	
	pixelSize = FreeImage_GetBPP(dib)/8;
	
	int size = width*height*pixelSize;
	bufptr = malloc( size );
	
	//swap R and B and invert image while copying
	byte* in, *out;
	for( int i = 0, ii = height-1; i < height ; ++i, --ii )
	{
		for( int j = 0; j < width; ++j )
		{
			out = (byte*)bufptr + (j + i*width)*pixelSize;
			in = (byte*)data + (j + ii*width)*pixelSize;

			if( pixelSize >= 4 )
				out[3] = in[3];
			
			if( pixelSize >= 3 )
			{
				out[2] = in[0];
				out[1] = in[1];
				out[0] = in[2];
			}
			else
			{
				out[0] = in[0];
			}
		}
	}
	
	FreeImage_Unload( dib );

	static const GLenum formatsForSize[] = { GL_NONE, GL_UNSIGNED_BYTE, GL_RG, GL_RGB, GL_RGBA };
	return formatsForSize[ pixelSize ];
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