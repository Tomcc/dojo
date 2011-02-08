#include "stdafx.h"

#include "LinuxPlatform.h"

#include <Poco/DirectoryIterator.h>
#include <Freeimage.h>

#include <glu.h>

#include "Render.h"
#include "Game.h"
#include "Utils.h"
#include "Table.h"

using namespace Dojo;
using namespace OIS;


LinuxPlatform::LinuxPlatform() :
Platform(),
dragging( false ),
cursorPos( 0,0 ){

}

bool LinuxPlatform::_initialiseWindow(const std::string& windowCaption, uint w, uint h){
	dpy = XOpenDisplay(NULL);
	if(dpy == NULL)
		return false;
	root = DefaultRootWindow(dpy);
	vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL)
		return false;
	cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	swa.colormap = cmap;
	swa.event_make = ExposureMask | KeyPressMask;
	win = XCreateWindow(dpy, root, 200, 200, w, h, 0, vi->depth, InputOutput, vi->visual, CWColorMap | CWEventMask, &swa);

}

void LinuxPlatform::initialise()
{
	DEBUG_ASSERT( game );

//crea la directory utente se non esiste
	std::string userDir = _getUserDirectory() + "/" + game->getName();

	CreateDirectoryA( userDir.c_str(), NULL );

	if( !_initialiseWindow( game->getName(), 480, 320 ) )
		return;

	glewInit();

	render = new Render( width, height, 1, Render::RO_LANDSCAPE_LEFT );

	sound = new SoundManager();

//inizializza OIS per emulare il touch
//evita il grab del cursore
	OIS::ParamList params;
	params.insert( std::make_pair( "WINDOW", Utils::toString( (uint)hwnd ) ) );
	params.insert( std::make_pair( "w32_mouse", "DISCL_FOREGROUND" ) );
	params.insert( std::make_pair( "w32_mouse", "DISCL_NONEXCLUSIVE" ) );

	inputManager = InputManager::createInputSystem( params );

	if( inputManager->getNumberOfDevices( OISKeyboard ) > 0)
	{
		keys = (Keyboard*)inputManager->createInputObject( OISKeyboard, true );
		keys->setEventCallback( this );
	}

	if( inputManager->getNumberOfDevices( OISMouse ) > 0 )
	{
		mouse = (Mouse*)inputManager->createInputObject( OISMouse, true );
		mouse->setEventCallback( this );
		mouse->getMouseState().width = width;
		mouse->getMouseState().height = height;
	}

	input = new TouchSource();

	//start the game
	game->onBegin();
}


void LinuxPlatform::shutdown()
{
	// and a cheesy fade exit
	AnimateWindow( hwnd, 200, AW_HIDE | AW_BLEND );
}

void LinuxPlatform::acquireContext()
{
	wglMakeCurrent( hdc, hglrc );
}

void LinuxPlatform::present()
{
	SwapBuffers( hdc );
}

void LinuxPlatform::step( float dt )
{
	DEBUG_ASSERT( running );

	//cattura l'input prima del gameplay
	keys->capture();
	mouse->capture();

	game->onLoop( dt);

	render->render();
	
	sound->update( dt );

}

void LinuxPlatform::loop( float frameTime )
{
	frameTimer.reset();

	float dt;
	while( running )
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if( msg.message == WM_QUIT )
				running = false;

			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		dt = frameTimer.getElapsedTime();

		if( dt > frameTime )
		{
			frameTimer.reset();

			step( dt );
		}
	}

	shutdown();
}

bool LinuxPlatform::mousePressed( const MouseEvent& arg, MouseButtonID id )
{
	dragging = true;

	cursorPos.x = arg.state.X.abs;
	cursorPos.y = arg.state.Y.abs;

	input->_fireTouchBeginEvent( cursorPos );

	return true;
}

bool LinuxPlatform::mouseMoved( const MouseEvent& arg )
{
	if( dragging )
	{
		cursorPos.x = arg.state.X.abs;
		cursorPos.y = arg.state.Y.abs;

		input->_fireTouchMoveEvent( cursorPos );
	}
	return true;
}

bool LinuxPlatform::mouseReleased( const MouseEvent& arg, MouseButtonID id )
{
	dragging = false;

	cursorPos.x = arg.state.X.abs;
	cursorPos.y = arg.state.Y.abs;

	input->_fireTouchEndEvent( cursorPos );

	return true;
}

bool LinuxPlatform::keyPressed(const OIS::KeyEvent &arg)
{
	input->_fireKeyPressedEvent( (uint)arg.key );

	return true;
}

bool LinuxPlatform::keyReleased(const OIS::KeyEvent &arg)
{
	input->_fireKeyReleasedEvent( (uint)arg.key );

	return true;
}

std::string LinuxPlatform::_toNormalPath( const std::string& input )
{
	std::string path = input;

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

std::string LinuxPlatform::getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path )
{
	//semplicemente il path relativo all'exe
	return _toNormalPath( path ) + "/" + name + "." + type; 
}

bool LinuxPlatform::_hasExtension( const std::string& ext, const std::string& nameOrPath )
{
	return nameOrPath.size() > ext.size() && ext == nameOrPath.substr( nameOrPath.size() - ext.size() );
}

void LinuxPlatform::getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out )
{
	try
	{
		Poco::DirectoryIterator itr( path );

		std::string extension = "." + type;

		while( itr.name().size() )
		{
			const std::string& name = itr.name();

			if( _hasExtension( extension, name ) )
				out.push_back( _toNormalPath( itr->path() ) );

			++itr;
		}
	}
	catch (...)
	{
		
	}	
}

uint LinuxPlatform::loadFileContent( char*& bufptr, const std::string& path )
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

void LinuxPlatform::loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height, bool POT )
{
	//puo' caricare tutto ma per coerenza meglio limitarsi alle PNG (TODO: usare freeimage su iPhone?)
	if( !_hasExtension( ".png", path ) )
		return;

	void* data;

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
	data = (void*)FreeImage_GetBits(dib);
	//get the image width and height, and size per pixel
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);

	uint realWidth = (POT) ? Math::nextPowerOfTwo( width ) : width;
	uint realHeight = (POT) ? Math::nextPowerOfTwo( height ) : height;
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

std::string LinuxPlatform::_getUserDirectory()
{
	char szPath[MAX_PATH];

	SHGetFolderPathA(
		hwnd, 
		CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE, 
		NULL, 
		0, 
		szPath);

	std::string dir( szPath );

	return _toNormalPath( dir );
}

void LinuxPlatform::load( Table* dest )
{
	DEBUG_ASSERT( dest );

	using namespace std;
	
	//cerca tra le user prefs un file con lo stesso nome
	string fileName = _getUserDirectory() + "/" + game->getName() + "/" + dest->getName() + ".txt";

	fstream file( fileName.c_str(), ios_base::in );

	if( !file.is_open() )
		return;

	dest->deserialize( file );

	file.close();
}

void LinuxPlatform::save( Table* table )
{
	DEBUG_ASSERT( table );

	using namespace std;
	
	string fileName = _getUserDirectory() + "/" + game->getName() + "/" + table->getName() + ".txt";

	fstream file( fileName.c_str(), ios_base::out | ios_base::trunc );

	if( !file.is_open() )
		return;

	table->serialize( file );

	file.close();
}

void LinuxPlatform::openWebPage( const std::string& site )
{
	ShellExecuteA(hwnd, "open", site.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
