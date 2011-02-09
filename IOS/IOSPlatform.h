#ifndef IOSPlatform_h__
#define IOSPlatform_h__

#include "dojo_common_header.h"

#include "Platform.h"
#include "Utils.h"

#ifdef __OBJC__
	#import <UIKit/UIKit.h>

	#import <OpenGLES/EAGL.h>
	#import <OpenGLES/EAGLDrawable.h>

	#import "Application.h"
#endif

namespace Dojo
{
	class IOSPlatform : public Platform
	{
	public:
		
		IOSPlatform();
		
		virtual ~IOSPlatform();
				
		virtual void initialise();
		virtual void shutdown();

		virtual void acquireContext();
		virtual void present();

		virtual void step( float dt );
		virtual void loop( float frameTime );

		virtual std::string getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path );
		virtual void getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out );
		virtual uint loadFileContent( char*& bufptr, const std::string& path );
		virtual void loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height );
		
		virtual uint loadAudioFileContent( ALuint& buffer, const std::string& path );
		
		virtual void load( Table* dest );
		virtual void save( Table* table );

		virtual void openWebPage( const std::string& site );
		
		virtual bool isSystemSoundInUse();
		
#ifdef __OBJC__
		void _notifyNativeApp( Application* application )
		{
			DEBUG_ASSERT( application );
			
			app = application;
		}
		
		inline Application* getNativeApplication()
		{			
			return app;
		}
#endif
		
	protected:
			
#ifdef __OBJC__
		NSAutoreleasePool * pool;
		
		EAGLContext* context;
		
		Application* app;		
		
		GLuint defaultFramebuffer, colorRenderbuffer;
#endif
		
	};
}

#endif