#ifndef IOSPlatform_h__
#define IOSPlatform_h__

#include "dojo_common_header.h"

#include "Platform.h"
#include "Utils.h"

#ifdef __OBJC__
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>

#endif

namespace Dojo
{
	class IOSPlatform : public Platform
	{
	public:
		
		inline void* getNativeApplication()
		{
			DEBUG_ASSERT( app );
			
			return app;
		}
		
		void bindColorBufferToContext( void* layer );
		
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
		
		void _notifyNativeApp( void* application )
		{
			app = application;
		}
		
	protected:
			
#ifdef __OBJC__
		EAGLContext* context;
#endif
		
		GLuint defaultFramebuffer, colorRenderbuffer;
		
		void* app;
	};
}

#endif