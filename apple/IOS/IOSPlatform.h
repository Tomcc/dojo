#ifndef IOSPlatform_h__
#define IOSPlatform_h__

#include "dojo_common_header.h"

#include "ApplePlatform.h"
#include "Utils.h"

#ifdef __OBJC__
	#import <UIKit/UIKit.h>

	#import <OpenGLES/EAGL.h>
	#import <OpenGLES/EAGLDrawable.h>

	#import "Application.h"
#endif

namespace Dojo
{
	class Texture;
	
	class IOSPlatform : public ApplePlatform
	{
	public:
		
		IOSPlatform( const Table& config );
		
		virtual ~IOSPlatform();
				
		virtual void initialise();
		virtual void shutdown();
		
		virtual void prepareThreadContext();
		
		virtual void acquireContext();
		virtual void present();

		virtual void loop( float frameTime );
				
		virtual bool isSystemSoundInUse();
				
		void copyImageIntoCameraRoll( Texture* tex );
				
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
		EAGLContext* context;
		
		Application* app;		
		
		GLuint defaultFramebuffer, colorRenderbuffer, depthRenderbuffer;
#endif
		
	};
}

#endif