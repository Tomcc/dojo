#include "stdafx.h"

#include "SoundBuffer.h"
#include "SoundManager.h"

#include "Utils.h"

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

using namespace Dojo;

#define OGG_ENDIAN 0

///////////////////////////////////////

SoundBuffer::SoundBuffer( ResourceGroup* creator, const String& path ) :
Buffer( creator, path ),
size(0),
buffer( AL_NONE )
{
	DEBUG_ASSERT( creator );
}

SoundBuffer::~SoundBuffer()
{
	unload();
}

bool SoundBuffer::load()
{
	if( isLoaded() )	return false;

	alGenBuffers(1,&buffer);
	
	DEBUG_ASSERT( buffer );

	ALenum error = alGetError();
	
	String ext = Utils::getFileExtension( filePath );
	
	DEBUG_ASSERT( ext == String( "ogg" ) );
	
	size = _loadOggFromFile();
			
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

////-------------------------------------////-------------------------------------////-------------------------------------

int SoundBuffer::_loadOggFromMemory( void * buf, int sz )
{
	VorbisSource src( buf, sz );
	ov_callbacks callbacks;
	
	callbacks.read_func = VorbisSource::read;
	callbacks.seek_func = VorbisSource::seek;
	callbacks.close_func = VorbisSource::close;
	callbacks.tell_func = VorbisSource::tell;
		
	OggVorbis_File file;
	vorbis_info* info;
	ALenum format;
	int uncompressedSize;
	
	int error = ov_open_callbacks( &src, &file, NULL, 0, callbacks );
	
	DEBUG_ASSERT( error == 0 );
	
	info = ov_info( &file, -1 );
	
	int wordSize = 2;
	format = (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	
	//guess uncompressed size (file should have only 1 stream)
	uncompressedSize = info->channels * ov_pcm_total( &file, -1 );
	
	char* uncompressedData = (char*)malloc( uncompressedSize );
	int section = -1;
	int totalRead = 0;
	
	//read all vorbis packets
	do 
	{
		int read = ov_read( &file, uncompressedData + totalRead, 4096, 0, wordSize, 1, &section );
		
		DEBUG_ASSERT( read >= 0 );
		
		totalRead += read;
	}
	while( read > 0 && totalRead < uncompressedSize );
	
	DEBUG_ASSERT( totalRead > 0 );
	
	alBufferData( buffer, format, uncompressedData, uncompressedSize, info->rate );
	
	DEBUG_ASSERT( alGetError() == AL_NO_ERROR );
	
	free( uncompressedData );
	
	return uncompressedSize;
}

int SoundBuffer::_loadOggFromFile()
{
	char* buf;
	int sz = Platform::getSingleton()->loadFileContent( buf, filePath );
	
	DEBUG_ASSERT( sz );
	
	_loadOggFromMemory( buf, sz );
	
	free( buf );
	
	return sz;
}

////-------------------------------------////-------------------------------------////-------------------------------------

void SoundBuffer::unload()
{
	if( isLoaded() )
	{
		alDeleteBuffers(1, &buffer);
		buffer = AL_NONE;
	}
}