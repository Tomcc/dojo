#include "Platform.h"

#include "Utils.h"
#include "dojomath.h"

using namespace Dojo;

void IOSPlatform::initialise()
{
	context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
	
    if (!context || ![EAGLContext setCurrentContext:context])
	{
		valid = false;
		return;
    }
	
	uint devicePixelScale = [UIScreen mainScreen].scale;
	uint width = [UIScreen mainScreen].bounds.size.height;
	uint height = [UIScreen mainScreen].bounds.size.width;
	
	render = new Render( this, width, height, devicePixelScale );
	
	
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
}

void IOSPlatform::shutdown()
{
	if( render )
	{
		delete render;
		
		// Tear down context
		if ([EAGLContext currentContext] == context)
			[EAGLContext setCurrentContext:nil];
		
		[context release];	
	}
}

void IOSPlatform::acquireContext()
{
	[EAGLContext setCurrentContext:context];
}

void IOSPlatform::present()
{
	[context presentRenderbuffer:GL_RENDERBUFFER];
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
	imageData = NULL;
	
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

void IOSPlatform::openWebPage( const std::string& site )
{
	NSString* url = Utils::toNSString( site );
	
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:url]];
}