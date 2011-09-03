//
//  ApplePlatform.cpp
//  dojo
//
//  Created by Tommaso Checchi on 5/16/11.
//  Copyright 2011 none. All rights reserved.
//

#include "ApplePlatform.h"

#import <AudioToolbox/AudioToolbox.h>
#import <Foundation/NSLocale.h>

#ifdef PLATFORM_IOS
	#import <UIKit/UIApplication.h>
#endif

#include "Timer.h"
#include "Render.h"
#include "SoundManager.h"
#include "InputSystem.h"
#include "Game.h"
#include "Utils.h"
#include "Table.h"
#include "dojostring.h"
#include "StringReader.h"

using namespace Dojo;
using namespace std;

ApplePlatform::ApplePlatform( const Table& config ) :
Platform( config ),
testSuite( std::cout )
{
    pool = [[NSAutoreleasePool alloc] init];
	
	locale = String( [[NSLocale preferredLanguages ] objectAtIndex:0] );
}

ApplePlatform::~ApplePlatform()
{
	[pool release];
}

void ApplePlatform::step( float dt )
{
	Timer frameTimer;
	
    game->loop(dt);
    
    render->render();
    sound->update(dt);
	
	realFrameTime = frameTimer.getElapsedTime();
}

String ApplePlatform::getCompleteFilePath( const String& name, const String& type, const String& path )
{
	NSString* NSName = name.toNSString();
	NSString* NSType = type.toNSString();
	NSString* NSPath = path.toNSString();
	
	NSString* res = [[NSBundle mainBundle] pathForResource:NSName ofType:NSType inDirectory:NSPath ];
		
	if( res )
		return String( res );
	else
		return String::EMPTY;
}


void ApplePlatform::getFilePathsForType( const String& type, const String& path, std::vector<String>& out )
{	
	DEBUG_ASSERT( type.size() );
	DEBUG_ASSERT( path.size() );
	
    NSString* nstype = type.toNSString();
	NSString* nspath = path.toNSString();
	
	NSArray* paths = [[NSBundle mainBundle] pathsForResourcesOfType:nstype inDirectory:nspath];
		
	//convert array
	for( uint i = 0; i < [paths count]; ++i )
	{
		out.push_back( String( [paths objectAtIndex:i] ) );
		
		DEBUG_MESSAGE( Utils::getFileName( out[i] ).ASCII() );
	}
}


NSString* ApplePlatform::_getFullPath( const String& path )
{	
	//is it already absolute?
	if( path[0] == '/' )
		return path.toNSString();
		
	NSString* nsbundlepath = [[NSBundle mainBundle] bundlePath];
	NSString* nspath = [nsbundlepath stringByAppendingString: ("/" + path).toNSString() ];
		
	return nspath;
}

String ApplePlatform::getAppDataPath()
{
	NSArray* nspaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString* nspath = [nspaths objectAtIndex:0];
	
	return String( nspath );
}

NSString* ApplePlatform::_getDestinationFilePath( Table* table, const String& absPath )
{	
	DEBUG_ASSERT( table );
	DEBUG_ASSERT( absPath.size() == 0 || absPath.at( absPath.size()-1 ) != '/' ); //need to NOT have the terminating /
	
	NSString* fullPath;	
	
	//save this in a file in the user preferences folder, or in the abs path
	if( absPath.size() == 0 )
	{		
		String fileName = "/" + game->getName() + "/" + table->getName() + ".ds";
		
		fullPath = (getAppDataPath() + fileName).toNSString();
	}
	else
	{		
		fullPath = _getFullPath( absPath.toNSString() );
	}
	
	return fullPath;
}

void ApplePlatform::load( Table* dest, const String& absPath )
{
	NSString* fullPath = _getDestinationFilePath( dest, absPath );
	
	//read file
	NSData* nsfile = [[NSData alloc] initWithContentsOfFile:fullPath];
	
	//do nothing if file doesn't exist
	if( !nsfile )
		return;
	
	//drop the data in a String - TODO don't duplicate it
	String buf;
	buf.appendRaw( (char*)[nsfile bytes], [nsfile length] );
	
	dest->setName( Utils::getFileName( String( fullPath ) ) );
	
	StringReader in( buf );
	dest->deserialize( in );
	
	[nsfile release];
}

void ApplePlatform::save( Table* src, const String& absPath )
{
	//serialize to stream
	String buf;
	src->serialize( buf );
	
	//ensure application dir created
	_createApplicationDirectory();
	
	NSString* fullPath = _getDestinationFilePath( src, absPath );
	
	//drop into NSData
	NSData* nsfile = [[NSData alloc] 	initWithBytesNoCopy:(void*)buf.data()
												   length:buf.byteSize()
											 freeWhenDone:false ];
	
	//drop on file
	[nsfile writeToFile:fullPath atomically:true];
	
	[nsfile release];
}


uint ApplePlatform::loadFileContent( char*& bufptr, const String& path )
{
    bufptr = NULL;
	
	DEBUG_ASSERT( path.size() );
		
	NSData* data = [ [NSData alloc ] initWithContentsOfFile: _getFullPath( path ) ];
	
	if( !data )
		return false;
	
	uint size = (uint)[data length];
	
	//alloc the new buffer
	bufptr = (char*)malloc( size );
	memcpy( bufptr, [data bytes], size );
	
	[data release];
	
	return size;
}


void ApplePlatform::loadPNGContent( void*& imageData, const String& path, uint& width, uint& height )
{
	width = height = 0;
	
	NSString* imgPath = path.toNSString();
		
	//magic OBJC code
	NSData *texData = [[NSData alloc] initWithContentsOfFile: imgPath ];
	
	DEBUG_ASSERT( texData );
	
	CGDataProviderRef prov = CGDataProviderCreateWithData( 
														  NULL, 
														  [texData bytes], 
														  [texData length], 
														  NULL );
	CGImageRef CGImage = CGImageCreateWithPNGDataProvider( prov, NULL, false, kCGRenderingIntentDefault );	
	
	width = (int)CGImageGetWidth(CGImage);
	height = (int)CGImageGetHeight(CGImage);	
	
	uint internalWidth = Math::nextPowerOfTwo( width );
	uint internalHeight = Math::nextPowerOfTwo( height );
	uint pitch = 4 * internalWidth;	
	
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	imageData = malloc( internalWidth * pitch );
	
	memset( imageData, 0, internalWidth * pitch );
	
	CGContextRef context = CGBitmapContextCreate(imageData, 
												 internalWidth, 
												 internalHeight, 
												 8, 
												 pitch, 
												 colorSpace, 
												 kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big );
	
	CGContextClearRect( context, CGRectMake( 0, 0, internalWidth, internalHeight ) );
	CGContextTranslateCTM( context, 0, internalHeight - height );
	CGContextDrawImage( context, CGRectMake( 0, 0, width, height ), CGImage );
	
	//correct premultiplied alpha - only in the useful part of the image
	byte* ptr = (byte*)imageData;
	byte* rowptr = (byte*)imageData;
	byte r, g, b;
	for( uint i = 0; i < height; ++i )
	{
		ptr = rowptr;
		
		for( uint j = 0; j < width; ++j )
		{
			byte alpha = ptr[3];
				
			if( alpha < 5 )
			{
				ptr[0] = r;
				ptr[1] = g;
				ptr[2] = b;
			}
			else if( alpha < 255 )
			{
				float a = ((float)(alpha)/255.f);
				
				r = (ptr[0] = ((float)ptr[0]) / a);
				g = (ptr[1] = ((float)ptr[1]) / a);
				b = (ptr[2] = ((float)ptr[2]) / a);
			}
			
			ptr += 4;
		}
		
		rowptr += pitch;
	}
	
	//free everything
	CGContextRelease(context);	
	CGColorSpaceRelease( colorSpace );
	CGImageRelease( CGImage );
	CGDataProviderRelease( prov );
	
	[texData release];
}


uint ApplePlatform::loadAudioFileContent( ALuint& buffer, const String& path )
{
	//only load caf files on iphone
	DEBUG_ASSERT( Utils::hasExtension( "caf", path ) );
	
	AudioFileID fileID;
	UInt64 dataSize;
	AudioStreamBasicDescription desc;
	UInt32 dataSize32;
	void* outData;
	OSStatus result;
	ALenum channelFormat;
	
	NSURL * afUrl = [NSURL fileURLWithPath: path.toNSString() ];
	
	//open file
	result = AudioFileOpenURL((CFURLRef)afUrl, kAudioFileReadPermission, 0, &fileID);
	
	//read format
	UInt32 propSize = sizeof( desc );	
	AudioFileGetProperty( fileID, kAudioFilePropertyDataFormat, &propSize, &desc );
	
	//read size
	propSize = sizeof( dataSize );
	AudioFileGetProperty( fileID, kAudioFilePropertyAudioDataByteCount, &propSize, &dataSize );
	dataSize32 = (UInt32)dataSize;
		
	//ensure format
	DEBUG_ASSERT( desc.mFormatID == kAudioFormatLinearPCM );	
	DEBUG_ASSERT( !((desc.mChannelsPerFrame > 2) || (desc.mChannelsPerFrame < 1) ) );
	
	//deduce channel format	
	if(desc.mBitsPerChannel == 8)
		channelFormat = (desc.mChannelsPerFrame == 1) ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
	else if(desc.mBitsPerChannel == 16)
		channelFormat = (desc.mChannelsPerFrame == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	else
	{
		DEBUG_TODO;
	}
	
	//read data
	outData = malloc( dataSize );
		
	AudioFileReadBytes( fileID, false, 0, &dataSize32, outData );
	
	//ensure native endianness and spit a warning if wrong!!
	if( desc.mBitsPerChannel > 8 && !TestAudioFormatNativeEndian( desc ) )
	{
		DEBUG_MESSAGE( "Warning slow loading: sound file " << path.ASCII() << " has wrong endianness" );
		
		//flip every damn word
		unsigned char* ptr = (unsigned char*) outData;
		
		for( UInt64 i = 0; i < dataSize; i += 2 )
		{
			unsigned char tmp = ptr[i];
			ptr[i] = ptr[i+1];
			ptr[i+1] = tmp;
		}
	}
	
	//finally, drop data into the buffer
	alBufferData( buffer, channelFormat, outData, dataSize32, desc.mSampleRate );
	
	free( outData );
	
	return dataSize;
}

void ApplePlatform::_createApplicationDirectory()
{	
	String dirname = "/" + game->getName();
	
	NSArray* nspaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString* nspath = [nspaths objectAtIndex:0];
	NSString* nsgamedir = dirname.toNSString();	
	NSString* nsdir = [nspath stringByAppendingString:nsgamedir];
	
	//check if the directory exists, if not existing create it!
	NSFileManager *fileManager= [NSFileManager defaultManager]; 
	if(![ fileManager fileExistsAtPath:nsdir ])
	{
		bool success = [fileManager 
						createDirectoryAtPath:nsdir
						withIntermediateDirectories:YES 
						attributes:nil 
						error:NULL];
		
		DEBUG_ASSERT( success );
	}	
}

void ApplePlatform::openWebPage( const String& site )
{
	NSURL* url = [NSURL URLWithString: site.toNSString() ];
	
#ifdef PLATFORM_OSX
	[[NSWorkspace sharedWorkspace] openURL: url ];
#else
	[[UIApplication sharedApplication] openURL:url];
#endif
}