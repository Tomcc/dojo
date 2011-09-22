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


size_t SoundBuffer::VorbisSource::read( void* out, size_t size, size_t count, void* source )
{
	VorbisSource* src = (VorbisSource*)source;

	int bytes = size * count;
	int max = src->size - src->pointer;

	if( bytes > max ) bytes = max;

	if( bytes > 0 )
	{
		memcpy( out, (char*)src->data + src->pointer, bytes );

		src->pointer += bytes;
	}

	return bytes / size;
}

int SoundBuffer::VorbisSource::seek( void *source, ogg_int64_t offset, int whence )
{
	VorbisSource* src = (VorbisSource*)source;

	if( whence == SEEK_SET )
		src->pointer = offset;
	else if( whence == SEEK_END )
		src->pointer = src->size - offset;
	else if( whence == SEEK_CUR )
		src->pointer += offset;
	else
	{
		DEBUG_TODO;
	}

	return 0;
}

int SoundBuffer::VorbisSource::close( void *source )
{
	return 0;
}

long SoundBuffer::VorbisSource::tell( void *source )
{
	return ((VorbisSource*)source)->pointer;
}

int SoundBuffer::_loadOggFromMemory( void * buf, int sz )
{
	VorbisSource src( buf, sz );
	ov_callbacks callbacks;
		
	OggVorbis_File file;
	vorbis_info* info;
	ALenum format;
	int uncompressedSize;

	callbacks.read_func = VorbisSource::read;
	callbacks.seek_func = VorbisSource::seek;
	callbacks.close_func = VorbisSource::close;
	callbacks.tell_func = VorbisSource::tell;

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
	int read = 0;
	do 
	{
		read = ov_read( &file, uncompressedData + totalRead, uncompressedSize-totalRead, 0, wordSize, 1, &section );
		
		if( read > 0 )
			totalRead += read;
	}
	while( read > 0 && totalRead < uncompressedSize );
	
	DEBUG_ASSERT( totalRead > 0 );
	
	alBufferData( buffer, format, uncompressedData, totalRead, info->rate );
	
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