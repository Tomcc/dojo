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

	_initKeyMap();
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

	input->_fireKeyPressedEvent( 0, mKeyMap[ kc ] );
}

void Win32Platform::keyReleased( int kc )
{
	input->_fireKeyReleasedEvent( lastPressedText, mKeyMap[ kc ] );
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
	TCHAR szPath[MAX_PATH];

	SHGetFolderPathW(
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
	ShellExecuteW(hwnd, L"open", site.c_str(), NULL, NULL, SW_SHOWNORMAL);
}


//init key map
void Win32Platform::_initKeyMap()
{
	mKeyMap[ 0 ] = InputSystem::KC_UNASSIGNED;

	mKeyMap[ VK_ESCAPE ] = InputSystem::KC_ESCAPE;
	mKeyMap[ VK_BACK ] = InputSystem::KC_BACK;
	mKeyMap[ VK_TAB ] = InputSystem::KC_TAB;

	mKeyMap[ 0x30 ] = InputSystem::KC_0;
	mKeyMap[ 0x31 ] = InputSystem::KC_1;
	mKeyMap[ 0x32 ] = InputSystem::KC_2;
	mKeyMap[ 0x33 ] = InputSystem::KC_3;
	mKeyMap[ 0x34 ] = InputSystem::KC_4;
	mKeyMap[ 0x35 ] = InputSystem::KC_5;
	mKeyMap[ 0x36 ] = InputSystem::KC_6;
	mKeyMap[ 0x37 ] = InputSystem::KC_7;
	mKeyMap[ 0x38 ] = InputSystem::KC_8;
	mKeyMap[ 0x39 ] = InputSystem::KC_9;

	mKeyMap[ 0x41 ] = InputSystem::KC_A;
	mKeyMap[ 0x42 ] = InputSystem::KC_B;
	mKeyMap[ 0x43 ] = InputSystem::KC_C;
	mKeyMap[ 0x44 ] = InputSystem::KC_D;
	mKeyMap[ 0x45 ] = InputSystem::KC_E;
	mKeyMap[ 0x46 ] = InputSystem::KC_F;
	mKeyMap[ 0x47 ] = InputSystem::KC_G;
	mKeyMap[ 0x48 ] = InputSystem::KC_H;
	mKeyMap[ 0x49 ] = InputSystem::KC_I;
	mKeyMap[ 0x4A ] = InputSystem::KC_J;
	mKeyMap[ 0x4B ] = InputSystem::KC_K;
	mKeyMap[ 0x4C ] = InputSystem::KC_L;
	mKeyMap[ 0x4D ] = InputSystem::KC_M;
	mKeyMap[ 0x4E ] = InputSystem::KC_N;
	mKeyMap[ 0x4F ] = InputSystem::KC_O;
	mKeyMap[ 0x50 ] = InputSystem::KC_P;
	mKeyMap[ 0x51 ] = InputSystem::KC_Q;
	mKeyMap[ 0X52 ] = InputSystem::KC_R;
	mKeyMap[ 0X53 ] = InputSystem::KC_S;
	mKeyMap[ 0X54 ] = InputSystem::KC_T;
	mKeyMap[ 0X55 ] = InputSystem::KC_U;
	mKeyMap[ 0X56 ] = InputSystem::KC_V;
	mKeyMap[ 0X57 ] = InputSystem::KC_W;
	mKeyMap[ 0X58 ] = InputSystem::KC_X;
	mKeyMap[ 0X59 ] = InputSystem::KC_Y;
	mKeyMap[ 0X5A ] = InputSystem::KC_Z;

	mKeyMap[ VK_NUMPAD0 ] = InputSystem::KC_NUMPAD0;
	mKeyMap[ VK_NUMPAD1 ] = InputSystem::KC_NUMPAD1;
	mKeyMap[ VK_NUMPAD2 ] = InputSystem::KC_NUMPAD2;
	mKeyMap[ VK_NUMPAD3 ] = InputSystem::KC_NUMPAD3;
	mKeyMap[ VK_NUMPAD4 ] = InputSystem::KC_NUMPAD4;
	mKeyMap[ VK_NUMPAD5 ] = InputSystem::KC_NUMPAD5;
	mKeyMap[ VK_NUMPAD6 ] = InputSystem::KC_NUMPAD6;
	mKeyMap[ VK_NUMPAD7 ] = InputSystem::KC_NUMPAD7;
	mKeyMap[ VK_NUMPAD8 ] = InputSystem::KC_NUMPAD8;
	mKeyMap[ VK_NUMPAD9 ] = InputSystem::KC_NUMPAD9;

	mKeyMap[ VK_F1 ] = InputSystem::KC_F1;
	mKeyMap[ VK_F2 ] = InputSystem::KC_F2;
	mKeyMap[ VK_F3 ] = InputSystem::KC_F3;
	mKeyMap[ VK_F4 ] = InputSystem::KC_F4;
	mKeyMap[ VK_F5 ] = InputSystem::KC_F5;
	mKeyMap[ VK_F6 ] = InputSystem::KC_F6;
	mKeyMap[ VK_F7 ] = InputSystem::KC_F7;
	mKeyMap[ VK_F8 ] = InputSystem::KC_F8;
	mKeyMap[ VK_F9 ] = InputSystem::KC_F9;
	mKeyMap[ VK_F10 ] = InputSystem::KC_F10;
	mKeyMap[ VK_F11 ] = InputSystem::KC_F11;
	mKeyMap[ VK_F12 ] = InputSystem::KC_F12;
	mKeyMap[ VK_F13 ] = InputSystem::KC_F13;
	mKeyMap[ VK_F14 ] = InputSystem::KC_F14;
	mKeyMap[ VK_F15 ] = InputSystem::KC_F15;

	mKeyMap[ VK_OEM_MINUS ] = InputSystem::KC_MINUS;
	mKeyMap[ 0 ] = InputSystem::KC_EQUALS;

	mKeyMap[ 0 ] = InputSystem::KC_LBRACKET;
	mKeyMap[ 0 ] = InputSystem::KC_RBRACKET;
	mKeyMap[ VK_RETURN ] = InputSystem::KC_RETURN;
	mKeyMap[ VK_LCONTROL ] = InputSystem::KC_LCONTROL;

	mKeyMap[ 0 ] = InputSystem::KC_SEMICOLON;
	mKeyMap[ 0 ] = InputSystem::KC_APOSTROPHE;
	mKeyMap[ 0 ] = InputSystem::KC_GRAVE;
	mKeyMap[ VK_SHIFT ] = InputSystem::KC_LSHIFT;
	mKeyMap[ 0 ] = InputSystem::KC_BACKSLASH;

	mKeyMap[ 0 ] = InputSystem::KC_COMMA;
	mKeyMap[ VK_OEM_PERIOD ] = InputSystem::KC_PERIOD;
	mKeyMap[ 0 ] = InputSystem::KC_SLASH;
	mKeyMap[ VK_RSHIFT ] = InputSystem::KC_RSHIFT;
	mKeyMap[ VK_MULTIPLY ] = InputSystem::KC_MULTIPLY;
	mKeyMap[ VK_LMENU ] = InputSystem::KC_LEFT_ALT;
	mKeyMap[ VK_SPACE ] = InputSystem::KC_SPACE;
	mKeyMap[ VK_CAPITAL ] = InputSystem::KC_CAPITAL;


	mKeyMap[ VK_NUMLOCK ] = InputSystem::KC_NUMLOCK;
	mKeyMap[ VK_SCROLL ] = InputSystem::KC_SCROLL;


	mKeyMap[ VK_SUBTRACT ] = InputSystem::KC_SUBTRACT;
	mKeyMap[ VK_ADD ] = InputSystem::KC_ADD;
	mKeyMap[ VK_DECIMAL ] = InputSystem::KC_DECIMAL;

	mKeyMap[ VK_OEM_102 ] = InputSystem::KC_OEM_102;


	mKeyMap[ 0 ] = InputSystem::KC_KANA;
	mKeyMap[ 0 ] = InputSystem::KC_ABNT_C1;
	mKeyMap[ 0 ] = InputSystem::KC_CONVERT;
	mKeyMap[ 0 ] = InputSystem::KC_NOCONVERT;
	mKeyMap[ 0 ] = InputSystem::KC_YEN;
	mKeyMap[ 0 ] = InputSystem::KC_ABNT_C2;
	mKeyMap[ 0 ] = InputSystem::KC_NUMPADEQUALS;

	mKeyMap[ 0 ] = InputSystem::KC_PREVTRACK;
	mKeyMap[ 0 ] = InputSystem::KC_AT;
	mKeyMap[ 0 ] = InputSystem::KC_COLON;
	mKeyMap[ VK_CANCEL ] = InputSystem::KC_STOP;
	mKeyMap[ 0 ] = InputSystem::KC_NUMPADENTER;
	mKeyMap[ 0 ] = InputSystem::KC_RCONTROL;

	mKeyMap[ VK_VOLUME_MUTE ] = InputSystem::KC_MUTE;
	mKeyMap[ VK_VOLUME_DOWN ] = InputSystem::KC_VOLUMEDOWN;
	mKeyMap[ VK_VOLUME_UP ] = InputSystem::KC_VOLUMEUP;
	mKeyMap[ VK_OEM_COMMA ] = InputSystem::KC_NUMPADCOMMA;
	mKeyMap[ VK_DIVIDE ] = InputSystem::KC_DIVIDE;
	mKeyMap[ VK_RMENU ] = InputSystem::KC_RIGHT_ALT;
	mKeyMap[ VK_PAUSE ] = InputSystem::KC_PAUSE;

	mKeyMap[ VK_HOME ] = InputSystem::KC_HOME;
	mKeyMap[ VK_PRIOR] = InputSystem::KC_PGUP;
	mKeyMap[ VK_NEXT ] = InputSystem::KC_PGDOWN;
	mKeyMap[ VK_INSERT ] = InputSystem::KC_INSERT;
	mKeyMap[ VK_DELETE ] = InputSystem::KC_DELETE;

	mKeyMap[ VK_UP ] = InputSystem::KC_UP;
	mKeyMap[ VK_LEFT ] = InputSystem::KC_LEFT;
	mKeyMap[ VK_RIGHT ] = InputSystem::KC_RIGHT;
	mKeyMap[ VK_DOWN ] = InputSystem::KC_DOWN;

	mKeyMap[ VK_END ] = InputSystem::KC_END;

	mKeyMap[ VK_LWIN ] = InputSystem::KC_LWIN;
	mKeyMap[ VK_RWIN ] = InputSystem::KC_RWIN;
	mKeyMap[ VK_APPS ] = InputSystem::KC_APPS;
	mKeyMap[ VK_SLEEP ] = InputSystem::KC_SLEEP;
}