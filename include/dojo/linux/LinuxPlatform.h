#pragma once

#include "dojo_common_header.h"

#include <OIS/OIS.h>

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "Platform.h"
#include "Vector.h"

#include "Timer.h"

namespace Dojo
{
	class LinuxPlatform : public Platform, public OIS::MouseListener, public OIS::KeyListener
	{
	public:

		LinuxPlatform(const Table& table);

		virtual void initialise();
		virtual void shutdown();

		virtual void acquireContext();
		virtual void present();

		virtual void step( float dt );
		virtual void loop( float frameTime );

		virtual std::string getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path);
		virtual void getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out);
		virtual uint loadFileContent( char*& bufptr, const std::string& path);
        virtual uint loadAudioFileContent(ALuint& i, const std::string& name);
		virtual void loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height);
		
		virtual void load(  Table* dest );
		virtual void save( Table* table );

		virtual void openWebPage( const std::string& site );

		virtual bool mouseMoved( const OIS::MouseEvent& arg );
		virtual bool mousePressed( const OIS::MouseEvent& arg, OIS::MouseButtonID id );
		virtual bool mouseReleased(	const OIS::MouseEvent& arg, OIS::MouseButtonID id );

		virtual bool keyPressed(const OIS::KeyEvent &arg);
		virtual bool keyReleased(const OIS::KeyEvent &arg);
		
		virtual void prepareThreadContext();
		
		virtual void loadPNGContent( void*& bufptr, const String& path, int& width, int& height );
		
		virtual String getAppDataPath();
		virtual String getRootPath();
		
		virtual void openWebPage( const String& site );
		

	protected:

		Display                 *dpy;
		Window                  root;
		static const GLint      att[];
		XVisualInfo             *vi;
		Colormap                cmap;
		XSetWindowAttributes    swa;
		Window                  win;
		GLXContext              glc;
		XWindowAttributes       gwa;
		XEvent                  xev;

		int width, height;

		Timer frameTimer;

		OIS::InputManager* inputManager;
		OIS::Mouse* mouse;
		OIS::Keyboard* keys;

		Vector cursorPos;

		bool dragging;

		bool _hasExtension( const std::string& type, const std::string& nameOrPath );
		std::string _toNormalPath( const std::string& path );

		std::string _getUserDirectory();

		bool _initialiseWindow( const std::string& caption, uint w, uint h );

	private:
	};
}

