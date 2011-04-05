#include "IOSPlatform.h"

#include "TargetConditionals.h"

#import <Foundation/NSString.h>
#import <QuartzCore/QuartzCore.h>
#import <AudioToolbox/AudioToolbox.h>	
#import <UIKit/UIKit.h>

#include "Utils.h"
#include "dojomath.h"

#include "Render.h"
#include "SoundManager.h"
#include "TouchSource.h"
#include "Game.h"
#include "Table.h"

using namespace Dojo;

IOSPlatform::IOSPlatform() :
app( NULL )
{
    pool = [[NSAutoreleasePool alloc] init];
}

IOSPlatform::~IOSPlatform()
{
	[pool release];
}

void IOSPlatform::initialise()
{
	DEBUG_ASSERT( app );
	
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
	
//INPUT MANAGER

	input = new TouchSource();
	
	game->onBegin();
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

void IOSPlatform::step( float dt )
{	
	game->onLoop( dt );
	
	render->render();
	
	sound->update( dt );
}

void IOSPlatform::loop( float minstep )
{
	DEBUG_ASSERT( !"DO NOT USE loop() ON IPHONE" );
}

std::string IOSPlatform::getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path )
{
	NSString* NSName = Utils::toNSString( name );
	NSString* NSType = Utils::toNSString( type );
	NSString* NSPath = Utils::toNSString( path );
	
	NSString* res = [[NSBundle mainBundle] pathForResource:NSName ofType:NSType inDirectory:NSPath ];
	
	if( res )
		return Utils::toSTDString( res );
	else
		return "";
}

void IOSPlatform::getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out )
{
	DEBUG_ASSERT( type.size() );
	DEBUG_ASSERT( path.size() );
	
	NSString* NSType = Utils::toNSString( type );
	NSString* NSPath = Utils::toNSString( path );
	
	NSArray* paths = [[NSBundle mainBundle] pathsForResourcesOfType:NSType inDirectory:NSPath ];
	
	for( uint i = 0; i < [paths count]; ++i )
		out.push_back( Utils::toSTDString( (NSString*)[paths objectAtIndex:i] ) );
								  
}

uint IOSPlatform::loadFileContent( char*& bufptr, const std::string& path )
{
	bufptr = NULL;
	
	DEBUG_ASSERT( path.size() );
	
	NSString* NSPath = Utils::toNSString( path );
	
	NSData* data = [[NSData alloc] initWithContentsOfFile: NSPath ];
	
	if( !data )
		return false;
	
	uint size = [data length];
	
	//alloc the new buffer
	bufptr = (char*)malloc( size );
	memcpy( bufptr, [data bytes], size );
	
	[data release];
	
	return size;
}

void IOSPlatform::loadPNGContent( void*& imageData, const std::string& path, uint& width, uint& height )
{
	width = height = 0;
	
	NSString* imgPath = Utils::toNSString( path );
	//magic OBJC code
	NSData *texData = [[NSData alloc] initWithContentsOfFile: imgPath ];
	UIImage *image = [[UIImage alloc] initWithData:texData];
	
	if (image == nil)
		return;
	
	width = CGImageGetWidth(image.CGImage);
	height = CGImageGetHeight(image.CGImage);	
	
	uint internalWidth = Math::nextPowerOfTwo( width );
	uint internalHeight = Math::nextPowerOfTwo( height );
	
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	imageData = malloc( internalWidth * internalHeight * 4 );
	memset( imageData, 0, internalWidth * internalHeight * 4 );
	
	CGContextRef context = CGBitmapContextCreate(imageData, 
												 internalWidth, 
												 internalHeight, 
												 8, 
												 4 * internalWidth, 
												 colorSpace, 
												 kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big );
	CGColorSpaceRelease( colorSpace );
	CGContextClearRect( context, CGRectMake( 0, 0, internalWidth, internalHeight ) );
	CGContextTranslateCTM( context, 0, internalHeight - height );
	CGContextDrawImage( context, CGRectMake( 0, 0, width, height ), image.CGImage );
	
	CGContextRelease(context);
		
	//correct premultiplied alpha
	int pixelcount = internalWidth*internalHeight;
	unsigned char* off = (unsigned char*)imageData;
	for( int pi=0; pi<pixelcount; ++pi )
	{
		unsigned char alpha = off[3];
		if( alpha!=255 && alpha!=0 )
		{
			off[0] = ((int)off[0])*255/alpha;
			off[1] = ((int)off[1])*255/alpha;
			off[2] = ((int)off[2])*255/alpha;
		}
		off += 4;
	}
	
}

uint IOSPlatform::loadAudioFileContent( ALuint& buffer, const std::string& path )
{
	//only load caf files on iphone
	DEBUG_ASSERT( Utils::hasExtension( "caf", path ) );
	
	NSString* filePath = Utils::toNSString( path );
	
	// first, open the file	
	AudioFileID fileID;
	// use the NSURl instead of a cfurlref cuz it is easier
	NSURL * afUrl = [NSURL fileURLWithPath:filePath];
	
	OSStatus result = AudioFileOpenURL((CFURLRef)afUrl, kAudioFileReadPermission, 0, &fileID);
	
	UInt64 outDataSize; 
	UInt32 propertySize, writable;
	
	AudioFileGetPropertyInfo( fileID, kAudioFilePropertyAudioDataByteCount, &propertySize, &writable );
	AudioFileGetProperty( fileID, kAudioFilePropertyAudioDataByteCount, &propertySize, &outDataSize);
	UInt32 fileSize = (UInt32)outDataSize;
	
	UInt32 freq;
	
	AudioFileGetPropertyInfo( fileID, kAudioFilePropertyBitRate, &propertySize, &writable );
	AudioFileGetProperty( fileID, kAudioFilePropertyBitRate, &propertySize, &freq );
	
	// this is where the audio data will live for the moment
	void* outData = malloc(fileSize);
	
	// this where we actually get the bytes from the file and put them
	// into the data buffer
	result = AudioFileReadBytes(fileID, false, 0, &fileSize, outData);
	AudioFileClose(fileID); //close the file
	
	// jam the audio data into the new buffer
	alBufferData( buffer, AL_FORMAT_STEREO16, outData, fileSize, freq/32); 
	
	free( outData );
	
	return fileSize;
}

void IOSPlatform::load( Dojo::Table * table, const std::string& relPath )
{
	DEBUG_ASSERT( table );
	
	if( relPath.size() )
		DEBUG_TODO;
			
	NSString* key = Utils::toNSString( table->getName() );
	
	NSData* data = [ [NSUserDefaults standardUserDefaults] dataForKey:key ];
	
	if( !data )
		return;
																	   
	std::stringstream str;
	
	str.write( (char*)[data bytes], [data length] );
	
	table->deserialize( str );
}

void IOSPlatform::save( Dojo::Table* table, const std::string& relPath )
{
	DEBUG_ASSERT( table );
	
	if( relPath.size() )
		DEBUG_TODO;
	
	std::stringstream str;
	
	table->serialize( str );
	
	uint size = str.str().size(); //nullchar
	
	str.seekg(0);
	char* chars = (char*)malloc( size );
	
	str.read( chars, size );
	
	NSData* data = [NSData dataWithBytesNoCopy: chars length: size ];
	NSString* name = Utils::toNSString( table->getName() );
			
	[[NSUserDefaults standardUserDefaults] setObject: data forKey: name ];
	
	[[NSUserDefaults standardUserDefaults] synchronize];
}

void IOSPlatform::openWebPage( const std::string& site )
{
	NSString* url = Utils::toNSString( site );
	
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:url]];
}

bool IOSPlatform::isSystemSoundInUse()
{
	UInt32 otherAudioIsPlaying;
	UInt32 size = sizeof(otherAudioIsPlaying);
	AudioSessionGetProperty(kAudioSessionProperty_OtherAudioIsPlaying, &size, &otherAudioIsPlaying);
	
	return otherAudioIsPlaying;
}