#include "stdafx.h"

#include "SoundBuffer.h"

#include "SoundManager.h"
#include "Utils.h"
#include "Platform.h"

using namespace Dojo;

#define OGG_ENDIAN 0

///////////////////////////////////////

SoundBuffer::SoundBuffer( ResourceGroup* creator, const String& path ) :
Resource( creator, path ),
size(0),
buffer( AL_NONE ),
mDuration( 0 )
{
	DEBUG_ASSERT( creator );
}

SoundBuffer::~SoundBuffer()
{

}

bool SoundBuffer::onLoad()
{
	DEBUG_ASSERT( isLoaded() == false );

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


void SoundBuffer::onUnload(bool soft)
{
	DEBUG_ASSERT( isLoaded() );

	if( !soft || isReloadable() )
	{
		alDeleteBuffers(1, &buffer);
		buffer = AL_NONE;
	}
}

////-------------------------------------////-------------------------------------////------------------------------------

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
		src->pointer = (long)offset;
	else if( whence == SEEK_END )
		src->pointer = src->size - (long)offset;
	else if( whence == SEEK_CUR )
		src->pointer += (long)offset;
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
	
	callbacks.read_func = VorbisSource::read;
	callbacks.seek_func = VorbisSource::seek;
	callbacks.close_func = VorbisSource::close;
	callbacks.tell_func = VorbisSource::tell;
		
	OggVorbis_File file;
	vorbis_info* info;
	ALenum format;
	int uncompressedSize, totalRead = 0;
	
	int error = ov_open_callbacks( &src, &file, NULL, 0, callbacks );
	
	DEBUG_ASSERT( error == 0 );
	
	info = ov_info( &file, -1 );
	
	int wordSize = 2;
	format = (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	
	int bitrate = info->rate;// * info->channels;
	int pcm = (int)ov_pcm_total( &file, -1 );
	uncompressedSize = pcm * wordSize * info->channels;

	mDuration = (float)pcm / (float)info->rate;

	char* uncompressedData = (char*)malloc( uncompressedSize );
	int section = -1;

	//read all vorbis packets in the same buffer (TODO: don't allocate the whole compressed buffer)
	long read = 0;

	bool corrupt = false;
	do
	{
		read = ov_read( &file, uncompressedData + totalRead, uncompressedSize - totalRead, 0, wordSize, 1, &section );

		if( read == OV_HOLE || read == OV_EBADLINK || read == OV_EINVAL )
			corrupt = true;

		else if( read == 0 )
			break;

		else
			totalRead += read;

		DEBUG_ASSERT( totalRead <= uncompressedSize ); //this should always be true

	} while( !corrupt );
	
	DEBUG_ASSERT( totalRead > 0 );
	
	alBufferData( buffer, format, uncompressedData, totalRead, bitrate );
	
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
