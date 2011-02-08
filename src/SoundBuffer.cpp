#include "stdafx.h"

#include "SoundBuffer.h"
#include "SoundManager.h"

#include "Utils.h"


using namespace Dojo;
using namespace std;

#define OGG_ENDIAN 0

///////////////////////////////////////

SoundBuffer::SoundBuffer( ResourceGroup* creator, const string& path ) :
Buffer( creator, path ),
size(0),
freq(0),
buffer( AL_NONE )
{
	DEBUG_ASSERT( creator );
}

SoundBuffer::~SoundBuffer()
{
	unload();
}

void SoundBuffer::_loadCAFBuffer()
{	
#ifndef PLATFORM_IOS
	DEBUG_TODO;
#else
	NSString* filePath = Utils::toNSString(filePath);
	
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
#endif
}

void SoundBuffer::_loadWAVBuffer()
{		
#ifdef PLATFORM_IOS
	DEBUG_TODO;
#else
	ALvoid* data;
	ALboolean loop;
	ALenum format;

	alutLoadWAVFile( (ALbyte*)filePath.c_str(), &format, &data, &size, &freq, &loop );

	alBufferData(buffer,format,data,size,freq);

	alutUnloadWAV(format,data,size,freq);
#endif
}

bool SoundBuffer::load()
{
	if( isLoaded() )	return false;

	alGenBuffers(1,&buffer);
	
	DEBUG_ASSERT( buffer );

	ALenum error = alGetError();
	
	if(  Utils::hasExtension( "caf", filePath ) )
		_loadCAFBuffer();
	else if(  Utils::hasExtension( "wav", filePath ) )
		_loadWAVBuffer();
		
	//error check
	if( error == AL_NO_ERROR )
		error = alGetError();

	if( error != AL_NO_ERROR )
	{
		alDeleteBuffers(1, &buffer);
		buffer = AL_NONE;
	}

	return error != AL_NO_ERROR;
}

void SoundBuffer::unload()
{
	if( isLoaded() )
	{
		alDeleteBuffers(1, &buffer);
		buffer = AL_NONE;
	}
}