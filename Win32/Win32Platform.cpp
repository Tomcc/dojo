#include "stdafx.h"

#include "Win32Platform.h"

#include <Windows.h>
#include <ShellAPI.h>
#include <ShlObj.h>

#include <Poco/DirectoryIterator.h>
#include <Freeimage.h>
#include <al/alut.h>
#include <gl/glu.h>

#include <gl/wglext.h>

#include "dojo/Render.h"
#include "dojo/Game.h"
#include "dojo/Utils.h"
#include "dojo/Table.h"
#include "dojo/FontSystem.h"

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

Win32Platform::Win32Platform() :
Platform(),
dragging( false ),
cursorPos( 0,0 ),
frameStart( 1 ),
frameInterval(0)
{
#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF |_CRTDBG_LEAK_CHECK_DF );
#endif

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
	rect.top = 100;
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
}

void Win32Platform::initialise()
{
	DEBUG_ASSERT( game );

	//create user dir if not existing
	String userDir = _getUserDirectory() + '/' + game->getName();

	CreateDirectoryA( userDir.ASCII().c_str(), NULL );

	//just use the game's preferred settings
	if( !_initialiseWindow( game->getName(), game->getNativeWidth(), game->getNativeHeight() ) )
		return;

	glewInit();

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

String Win32Platform::_toNormalPath( const String& input )
{
	String path = input;

	for( uint i = 0; i < path.size(); ++i )
	{
		if( path[i] == '\\' )
			path[i] = '/';
	}

	//remove ending /
	if( path[path.size()-1] == '/')
		path.resize( path.size()-1 );

	return path;
}

String Win32Platform::getCompleteFilePath( const String& name, const String& type, const String& path )
{
	//semplicemente il path relativo all'exe
	return _toNormalPath( path ) + '/' + name + '.' + type; 
}

bool Win32Platform::_hasExtension( const String& ext, const String& nameOrPath )
{
	return nameOrPath.size() > ext.size() && ext == nameOrPath.substr( nameOrPath.size() - ext.size() );
}

void Win32Platform::getFilePathsForType( const String& type, const String& path, std::vector<String>& out )
{
	try
	{
		Poco::DirectoryIterator itr( path.ASCII() );

		String extension = '.' + type;

		while( itr.name().size() )
		{
			const String& name = itr.name();

			if( _hasExtension( extension, name ) )
				out.push_back( _toNormalPath( itr->path() ) );

			++itr;
		}
	}
	catch (...)
	{
		
	}	
}

uint Win32Platform::loadFileContent( char*& bufptr, const String& path )
{	
	using namespace std;

	fstream file( path.c_str(), ios_base::in | ios_base::ate );

	if( !file.is_open() )
		return 0;

	uint size = file.tellg();
	file.seekg(0);

	bufptr = (char*)malloc( size );

	file.read( bufptr, size );

	file.close();

	return size;
}

void Win32Platform::loadPNGContent( void*& bufptr, const String& path, uint& width, uint& height )
{
	//puo' caricare tutto ma per coerenza meglio limitarsi alle PNG (TODO: usare freeimage su iPhone?)
	if( !_hasExtension( ".png", path ) )
		return;

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
		return;

	//check that the plugin has reading capabilities and load the file
	if(FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, ansipath.c_str());
	//if the image failed to load, return failure
	if(!dib)
		return;

	//retrieve the image data
	data = (void*)FreeImage_GetBits(dib);
	//get the image width and height, and size per pixel
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);

	uint realWidth = Math::nextPowerOfTwo( width );
	uint realHeight = Math::nextPowerOfTwo( height );

	uint pixelSize = FreeImage_GetBPP(dib)/8;
	uint realSize = realWidth * realHeight * 4;

	bufptr = malloc( realSize );
	memset( bufptr, 0, realSize );

	//converti a 4 canali e scambia R e B
	byte* in = (byte*)data;
	byte* out = (byte*)bufptr;
	for( int i = height-1; i >= 0; --i )
	{
		for( uint j = 0; j < width*4; j += 4 )
		{
			out[ j + i*realWidth*4 + 3 ] = in[3];
 			out[ j + i*realWidth*4 + 2 ] = in[0];
			out[ j + i*realWidth*4 + 1 ] = in[1];
			out[ j + i*realWidth*4 + 0 ] = in[2];

			in += pixelSize;
		}
	}

	FreeImage_Unload( dib );
}

uint Win32Platform::loadAudioFileContent( ALuint& buffer, const String& filePath )
{
	ALvoid* data;
	ALboolean loop;
	ALenum format;
	ALsizei size, freq;

	alutLoadWAVFile( (ALbyte*)filePath.c_str(), &format, &data, &size, &freq, &loop );

	alBufferData(buffer,format,data,size,freq);

	alutUnloadWAV(format,data,size,freq);

	return size;
}

String Win32Platform::_getUserDirectory()
{
	char szPath[MAX_PATH];

	SHGetFolderPathA(
		hwnd, 
		CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE, 
		NULL, 
		0, 
		szPath);

	String dir( szPath );

	return _toNormalPath( dir );
}

String Win32Platform::_getFilename( Table* dest, const String& absPath )
{	
	if( absPath.size() == 0 )
	{
		DEBUG_ASSERT( dest->hasName() );

		//cerca tra le user prefs un file con lo stesso nome
		return _getUserDirectory() + '/' + game->getName() + '/' + dest->getName() + ".ds";
	}
	else
		return absPath;
}

void Win32Platform::load( Table* dest, const String& absPath )
{
	DEBUG_ASSERT( dest );
	using namespace std;

	String filename =  _getFilename(dest, absPath);
	FileStream file( filename.c_str(), ios_base::in | ios_base::binary );

	if( !file.is_open() )
		return;

	dest->setName( Utils::getFileName( filename ) );
	dest->deserialize( file );

	file.close();
}

void Win32Platform::save( Table* src, const String& absPath )
{
	DEBUG_ASSERT( src );
	using namespace std;
	
	FileStream file( _getFilename(src, absPath).c_str(), ios_base::out | ios_base::trunc | ios_base::binary );

	if( !file.is_open() )
		return;

	src->serialize( file );

	file.close();
}

void Win32Platform::openWebPage( const String& site )
{
	ShellExecuteA(hwnd, "open", site.ASCII().c_str(), NULL, NULL, SW_SHOWNORMAL);
}