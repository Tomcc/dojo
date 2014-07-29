#include "dojo_common_header.h"

#ifdef PLATFORM_LINUX

#include "Platform.h"
#include "Vector.h"

#include "Timer.h"

namespace Dojo
{
	class LinuxPlatform : public Platform
	{
	public:

		LinuxPlatform( const Table& config );
		
		///initializes the platform and calls Game::onBegin()
		virtual void initialize( Game* game );

		///shuts down the Platform and calls Game::onEnd()
		virtual void shutdown();
		
		///CALL THIS BEFORE USING ANY OTHER THREAD FOR GL OPERATIONS
		virtual void prepareThreadContext();

		///switches the game to windowed, if supported
		virtual void setFullscreen( bool enabled );

		virtual void acquireContext();
		virtual void present();

		virtual void step( float dt );
		virtual void loop();

		virtual GLenum loadImageFile( void*& bufptr, const String& path, int& width, int& height, int& pixelSize );

		///returns true if the device is able to manage non-power-of-2 textures
		virtual bool isNPOTEnabled();

		///returns the application data path for this game (eg. to save user files)
		virtual const String& getAppDataPath();
		///returns the read-only root path for this game (eg. working directory)
		virtual const String& getRootPath();
		///returns the read-only resources path, eg working directory on windows or Bundle/Contents/Resources on Mac
		virtual const String& getResourcesPath	();
		
		///opens a web page in the default browser
		virtual void openWebPage( const String& site );
		
	protected:

	private:
	};
}

