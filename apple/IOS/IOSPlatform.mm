#include "IOSPlatform.h"

#include "TargetConditionals.h"

#import <Foundation/NSString.h>
#import <QuartzCore/QuartzCore.h>
#import <AudioToolbox/AudioToolbox.h>	
#import <UIKit/UIKit.h>

#include "Utils.h"
#include "dojomath.h"

#include <dojo/Render.h>
#include <dojo/SoundManager.h>
#include <dojo/InputSystem.h>
#include <dojo/FontSystem.h>
#include <dojo/Game.h>
#include <dojo/Table.h>

using namespace Dojo;

IOSPlatform::IOSPlatform( const Table& config ) :
ApplePlatform( config ),
app( NULL )
{
	
}

IOSPlatform::~IOSPlatform()
{
	
}

void IOSPlatform::initialise()
{
	DEBUG_ASSERT( app );
	
	//if( config.exists( "runTests" ) )
	testSuite.staticTest();
	
	uint devicePixelScale = [UIScreen mainScreen].scale;
	uint width = [UIScreen mainScreen].bounds.size.height;
	uint height = [UIScreen mainScreen].bounds.size.width;
		
//RENDER
	context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
	
    if (!context || ![EAGLContext setCurrentContext:context] )
		return;
	
//on IOS the default target is always a separate renderbuffer
	
	CAEAGLLayer* layer = (CAEAGLLayer*)app.layer;	
	GLint w, h;
	
	glGenFramebuffers(1, &defaultFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
	
	glGenRenderbuffers(1, &colorRenderbuffer);	
	glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
	[context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &w);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &h);
	
	width = w;
	height = h;
	
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
	
	render = new Render( width, height, devicePixelScale, Render::RO_PORTRAIT );
		
	
//SOUND MANAGER

	AudioSessionInitialize ( NULL, NULL, 
							NULL, // may want interruption callback here
							NULL );
	
	//if using mp3 playback, it is needed to exclude other applications' sounds
#ifdef HARDWARE_SOUND
	UInt32 sessionCategory = kAudioSessionCategory_SoloAmbientSound;    // 1
#else
	uint sessionCategory = kAudioSessionCategory_AmbientSound;
#endif
	AudioSessionSetProperty (kAudioSessionProperty_AudioCategory, sizeof (sessionCategory), &sessionCategory);
	AudioSessionSetActive (true);	
	
	sound = new SoundManager();
	input = new InputSystem();
	fonts = new FontSystem();	
	
	if( config.exists( "runTests" ) )
		testSuite.initTests();
	
	game->begin();
	
	running = true;
	
	
	if( config.exists( "runTests" ) )
		testSuite.runtimeTests();
}
	
void IOSPlatform::shutdown()
{
	if( render )
	{
		delete render;
		
		// Tear down GL
		if (defaultFramebuffer)
		{
			glDeleteFramebuffers(1, &defaultFramebuffer);
			defaultFramebuffer = 0;
		}
		
		if (colorRenderbuffer)
		{
			glDeleteRenderbuffers(1, &colorRenderbuffer);
			colorRenderbuffer = 0;
		}
		
		// Tear down context
		if ([EAGLContext currentContext] == context)
			[EAGLContext setCurrentContext:nil];
		
		[context release];	
	}
}

void IOSPlatform::acquireContext()
{
	[EAGLContext setCurrentContext:context];
	
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
}

void IOSPlatform::present()
{	
	[EAGLContext setCurrentContext:context];
	
	glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
	
	[context presentRenderbuffer:GL_RENDERBUFFER_OES];
}

void IOSPlatform::loop( float minstep )
{
	DEBUG_TODO;
}

bool IOSPlatform::isSystemSoundInUse()
{
	UInt32 otherAudioIsPlaying;
	UInt32 size = sizeof(otherAudioIsPlaying);
	AudioSessionGetProperty(kAudioSessionProperty_OtherAudioIsPlaying, &size, &otherAudioIsPlaying);
	
	return otherAudioIsPlaying;
}