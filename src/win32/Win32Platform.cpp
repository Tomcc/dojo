#include "stdafx.h"

#include "win32/Win32Platform.h"
#include "win32/WGL_ARB_multisample.h"
#include "win32/XInputJoystick.h"

#include "Render.h"
#include "Game.h"
#include "Utils.h"
#include "Table.h"
#include "FontSystem.h"
#include "dojomath.h"
#include "SoundManager.h"
#include "InputSystem.h"

#include "Keyboard.h"

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
		app->mouseWheelMoved( (int)( (float)HIWORD(wparam) / (float)WHEEL_DELTA ) );
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


	case WM_SYSKEYDOWN:
		if( wparam == VK_F4 ) //listen for Alt+F4
		{
			PostQuitMessage(1);
			return 0;
		}

		//continues after the jump!!

    case WM_KEYDOWN:

		switch( wparam )
		{
		case VK_ESCAPE:
#ifdef _DEBUG  //close with ESC automagically
			PostQuitMessage( 0 );
			break;
#endif
		default:

			app->keyPressed( wparam );
			break;
		}
        return 0;

	case WM_SYSKEYUP: //needed to catch ALT separately
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

Win32Platform::Win32Platform( const Table& configTable ) :
Platform( configTable ),
dragging( false ),
mMousePressed( false ),
cursorPos( Vector::ZERO ),
frameStart( 1 ),
frameInterval(0),
mFramesToAdvance( 0 )
{
	/*
#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF |_CRTDBG_LEAK_CHECK_DF );
#endif
	*/

	//if no config is supplied, try to load it from an user file

	screenWidth = GetSystemMetrics( SM_CXSCREEN );
	screenHeight = GetSystemMetrics( SM_CYSCREEN );

	frameStart.wait();

	_initKeyMap();
}

Win32Platform::~Win32Platform()
{
	_setFullscreen( false ); //get out of fullscreen

	for( int i = 0; i < 4; ++i )
		delete mXInputJoystick[ i ];
}

void Win32Platform::_adjustWindow()
{
	RECT clientSize;
    //GetClientRect(HWnd, &clientSize);
    clientSize.top = 0;
    clientSize.left = 0;
    clientSize.right = windowWidth;
    clientSize.bottom = windowHeight;

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

	hwnd = CreateWindowW(L"DojoOpenGLWindow",
		windowCaption.c_str(),
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

	_setFullscreen( mFullscreen );

	return err;
}

void Win32Platform::_setFullscreen( bool fullscreen )
{
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
		dm.dmPelsWidth = windowWidth;
		dm.dmPelsHeight	= windowHeight;
		dm.dmBitsPerPel	= 32;
		dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		LONG ret = ChangeDisplaySettings( &dm, CDS_FULLSCREEN );

		DEBUG_ASSERT( ret == DISP_CHANGE_SUCCESSFUL );

		//WARNING MoveWindow can change backbuffer size
		MoveWindow(hwnd, 0, 0, dm.dmPelsWidth, dm.dmPelsHeight, TRUE);
	}

	ShowCursor( !fullscreen );
}

void Win32Platform::setFullscreen( bool fullscreen )
{
	if( fullscreen == mFullscreen )
		return;

	_setFullscreen( fullscreen );

	mFullscreen = fullscreen;

	//store the new setting into config.ds
	config.setBoolean( "fullscreen", mFullscreen );
	save( &config );
}

void Win32Platform::initialise( Game* g )
{
	game = g;
	DEBUG_ASSERT( game );

	DEBUG_MESSAGE( "Initializing Dojo Win32" );

	//create user dir if not existing
	String userDir = getAppDataPath() + '/' + game->getName();

	CreateDirectoryW( userDir.c_str(), NULL );

	//load settings
	if( config.isEmpty() )
		Table::loadFromFile( &config, getAppDataPath() + "/" + game->getName() + "/config.ds" );

	Vector windowSize = config.getVector("windowSize", Vector( (float)screenWidth, (float)screenHeight ) );
	windowWidth = (int)windowSize.x;
	windowHeight = (int)windowSize.y;

	mFullscreen = config.getBool( "fullscreen" );

	//just use the game's preferred settings
	if( !_initialiseWindow( game->getName(), windowWidth, windowHeight ) )
		return;

	setVSync( !config.getBool( "disable_vsync" ) );		
	
	render = new Render( width, height, DO_LANDSCAPE_LEFT );

	sound = new SoundManager();

	input = new InputSystem();

	//add the keyboard
	input->addDevice( &mKeyboard );
	//create xinput persistent joysticks
	for( int i = 0; i < 4; ++i ) 
	{
		mXInputJoystick[ i ] = new XInputJoystick( i );
		mXInputJoystick[i]->poll( 1 ); //force detection of already connected pads
	}

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
	if( game )
	{
		game->end();
		delete game;
	}

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
	//take the time before swapBuffers because on some implementations it is blocking
	realFrameTime = (float)mStepTimer.getElapsedTime();	

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

void Win32Platform::_pollDevices( float dt )
{
	mKeyboard.poll( dt );

	for( auto j : mXInputJoystick )
		j->poll( dt );

	//TODO DInput joysticks
}

void Win32Platform::step( float dt )
{
	mStepTimer.reset();
	
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

	//update input
	_pollDevices( dt );

	game->loop( dt);

	sound->update( dt );

	render->render();
}

void Win32Platform::loop()
{
	DEBUG_ASSERT( game );

	frameInterval = game->getNativeFrameLength();

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

		if( !mFrameSteppingEnabled || (mFrameSteppingEnabled && mFramesToAdvance > 0 ) )
		{
			step( dt );

			if( mFrameSteppingEnabled )
				--mFramesToAdvance;
		}
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

	Dojo::KeyCode key = mKeyMap[ kc ];

#ifndef _DEBUG
	if( key == KC_DIVIDE )	
		mFrameSteppingEnabled = !mFrameSteppingEnabled; 
	else if( mFrameSteppingEnabled )
	{
		if( key == KC_NUMPAD0 )		mFramesToAdvance = 1;
		else if( key == KC_NUMPAD1 )	mFramesToAdvance = 5;
		else if( key == KC_NUMPAD2 )	mFramesToAdvance = 10;
		else if( key == KC_NUMPAD3 )	mFramesToAdvance = 20;
		else if( key == KC_NUMPAD4 )	mFramesToAdvance = 50;
		else if( key == KC_NUMPAD5 )	mFramesToAdvance = 100;
		else if( key == KC_NUMPAD6 )	mFramesToAdvance = 200;
		else if( key == KC_NUMPAD7 )	mFramesToAdvance = 500;
	}
#endif
	
	mKeyboard._notifyButtonState( key, true );
}

void Win32Platform::keyReleased( int kc )
{
	mKeyboard._notifyButtonState( mKeyMap[ kc ], false );
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
		CSIDL_APPDATA|CSIDL_FLAG_CREATE, 
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

String Win32Platform::getResourcesPath()
{
	return getRootPath(); //on windows, it is the same
}

void Win32Platform::openWebPage( const String& site )
{
	ShellExecuteW(hwnd, L"open", site.c_str(), NULL, NULL, SW_SHOWNORMAL);
}


//init key map
void Win32Platform::_initKeyMap()
{
	ZeroMemory( mKeyMap, sizeof( mKeyMap ) );

	mKeyMap[ VK_ESCAPE ] = KC_ESCAPE;
	mKeyMap[ VK_BACK ] = KC_BACK;
	mKeyMap[ VK_TAB ] = KC_TAB;

	mKeyMap[ 0x30 ] = KC_0;
	mKeyMap[ 0x31 ] = KC_1;
	mKeyMap[ 0x32 ] = KC_2;
	mKeyMap[ 0x33 ] = KC_3;
	mKeyMap[ 0x34 ] = KC_4;
	mKeyMap[ 0x35 ] = KC_5;
	mKeyMap[ 0x36 ] = KC_6;
	mKeyMap[ 0x37 ] = KC_7;
	mKeyMap[ 0x38 ] = KC_8;
	mKeyMap[ 0x39 ] = KC_9;

	mKeyMap[ 0x41 ] = KC_A;
	mKeyMap[ 0x42 ] = KC_B;
	mKeyMap[ 0x43 ] = KC_C;
	mKeyMap[ 0x44 ] = KC_D;
	mKeyMap[ 0x45 ] = KC_E;
	mKeyMap[ 0x46 ] = KC_F;
	mKeyMap[ 0x47 ] = KC_G;
	mKeyMap[ 0x48 ] = KC_H;
	mKeyMap[ 0x49 ] = KC_I;
	mKeyMap[ 0x4A ] = KC_J;
	mKeyMap[ 0x4B ] = KC_K;
	mKeyMap[ 0x4C ] = KC_L;
	mKeyMap[ 0x4D ] = KC_M;
	mKeyMap[ 0x4E ] = KC_N;
	mKeyMap[ 0x4F ] = KC_O;
	mKeyMap[ 0x50 ] = KC_P;
	mKeyMap[ 0x51 ] = KC_Q;
	mKeyMap[ 0X52 ] = KC_R;
	mKeyMap[ 0X53 ] = KC_S;
	mKeyMap[ 0X54 ] = KC_T;
	mKeyMap[ 0X55 ] = KC_U;
	mKeyMap[ 0X56 ] = KC_V;
	mKeyMap[ 0X57 ] = KC_W;
	mKeyMap[ 0X58 ] = KC_X;
	mKeyMap[ 0X59 ] = KC_Y;
	mKeyMap[ 0X5A ] = KC_Z;

	mKeyMap[ VK_NUMPAD0 ] = KC_NUMPAD0;
	mKeyMap[ VK_NUMPAD1 ] = KC_NUMPAD1;
	mKeyMap[ VK_NUMPAD2 ] = KC_NUMPAD2;
	mKeyMap[ VK_NUMPAD3 ] = KC_NUMPAD3;
	mKeyMap[ VK_NUMPAD4 ] = KC_NUMPAD4;
	mKeyMap[ VK_NUMPAD5 ] = KC_NUMPAD5;
	mKeyMap[ VK_NUMPAD6 ] = KC_NUMPAD6;
	mKeyMap[ VK_NUMPAD7 ] = KC_NUMPAD7;
	mKeyMap[ VK_NUMPAD8 ] = KC_NUMPAD8;
	mKeyMap[ VK_NUMPAD9 ] = KC_NUMPAD9;

	mKeyMap[ VK_F1 ] = KC_F1;
	mKeyMap[ VK_F2 ] = KC_F2;
	mKeyMap[ VK_F3 ] = KC_F3;
	mKeyMap[ VK_F4 ] = KC_F4;
	mKeyMap[ VK_F5 ] = KC_F5;
	mKeyMap[ VK_F6 ] = KC_F6;
	mKeyMap[ VK_F7 ] = KC_F7;
	mKeyMap[ VK_F8 ] = KC_F8;
	mKeyMap[ VK_F9 ] = KC_F9;
	mKeyMap[ VK_F10 ] = KC_F10;
	mKeyMap[ VK_F11 ] = KC_F11;
	mKeyMap[ VK_F12 ] = KC_F12;
	mKeyMap[ VK_F13 ] = KC_F13;
	mKeyMap[ VK_F14 ] = KC_F14;
	mKeyMap[ VK_F15 ] = KC_F15;

	mKeyMap[ VK_OEM_MINUS ] = KC_MINUS;
	mKeyMap[ 0 ] = KC_EQUALS;

	mKeyMap[ 0 ] = KC_LBRACKET;
	mKeyMap[ 0 ] = KC_RBRACKET;
	mKeyMap[ VK_RETURN ] = KC_RETURN;
	mKeyMap[ VK_LCONTROL ] = KC_LCONTROL;

	mKeyMap[ 0 ] = KC_SEMICOLON;
	mKeyMap[ 0 ] = KC_APOSTROPHE;
	mKeyMap[ 0 ] = KC_GRAVE;
	mKeyMap[ VK_SHIFT ] = KC_LSHIFT;
	mKeyMap[ 0 ] = KC_BACKSLASH;

	mKeyMap[ 0 ] = KC_COMMA;
	mKeyMap[ VK_OEM_PERIOD ] = KC_PERIOD;
	mKeyMap[ 0 ] = KC_SLASH;
	mKeyMap[ VK_RSHIFT ] = KC_RSHIFT;
	mKeyMap[ VK_MULTIPLY ] = KC_MULTIPLY;
	mKeyMap[ 18 ] = KC_LEFT_ALT;
	mKeyMap[ VK_SPACE ] = KC_SPACE;
	mKeyMap[ VK_CAPITAL ] = KC_CAPITAL;


	mKeyMap[ VK_NUMLOCK ] = KC_NUMLOCK;
	mKeyMap[ VK_SCROLL ] = KC_SCROLL;


	mKeyMap[ VK_SUBTRACT ] = KC_SUBTRACT;
	mKeyMap[ VK_ADD ] = KC_ADD;
	mKeyMap[ VK_DECIMAL ] = KC_DECIMAL;

	mKeyMap[ VK_OEM_102 ] = KC_OEM_102;


	mKeyMap[ 0 ] = KC_KANA;
	mKeyMap[ 0 ] = KC_ABNT_C1;
	mKeyMap[ 0 ] = KC_CONVERT;
	mKeyMap[ 0 ] = KC_NOCONVERT;
	mKeyMap[ 0 ] = KC_YEN;
	mKeyMap[ 0 ] = KC_ABNT_C2;
	mKeyMap[ 0 ] = KC_NUMPADEQUALS;

	mKeyMap[ 0 ] = KC_PREVTRACK;
	mKeyMap[ 0 ] = KC_AT;
	mKeyMap[ 0 ] = KC_COLON;
	mKeyMap[ VK_CANCEL ] = KC_STOP;
	mKeyMap[ 0 ] = KC_NUMPADENTER;
	mKeyMap[ 0 ] = KC_RCONTROL;

	mKeyMap[ VK_VOLUME_MUTE ] = KC_MUTE;
	mKeyMap[ VK_VOLUME_DOWN ] = KC_VOLUMEDOWN;
	mKeyMap[ VK_VOLUME_UP ] = KC_VOLUMEUP;
	mKeyMap[ VK_OEM_COMMA ] = KC_NUMPADCOMMA;
	mKeyMap[ VK_DIVIDE ] = KC_DIVIDE;
	mKeyMap[ 17 ] = KC_RIGHT_ALT;
	mKeyMap[ VK_PAUSE ] = KC_PAUSE;

	mKeyMap[ VK_HOME ] = KC_HOME;
	mKeyMap[ VK_PRIOR] = KC_PGUP;
	mKeyMap[ VK_NEXT ] = KC_PGDOWN;
	mKeyMap[ VK_INSERT ] = KC_INSERT;
	mKeyMap[ VK_DELETE ] = KC_DELETE;

	mKeyMap[ VK_UP ] = KC_UP;
	mKeyMap[ VK_LEFT ] = KC_LEFT;
	mKeyMap[ VK_RIGHT ] = KC_RIGHT;
	mKeyMap[ VK_DOWN ] = KC_DOWN;

	mKeyMap[ VK_END ] = KC_END;

	mKeyMap[ VK_LWIN ] = KC_LWIN;
	mKeyMap[ VK_RWIN ] = KC_RWIN;
	mKeyMap[ VK_APPS ] = KC_APPS;
	mKeyMap[ VK_SLEEP ] = KC_SLEEP;

	mKeyMap[ 0 ] = KC_UNASSIGNED;
}