#include "stdafx.h"

#include "SoundBuffer.h"

#include "SoundManager.h"
#include "MemoryInputStream.h"
#include "FileStream.h"
#include "Utils.h"
#include "Platform.h"

using namespace Dojo;

#define OGG_ENDIAN 0

//store the callbacks
ov_callbacks SoundBuffer::VORBIS_CALLBACKS = 
{ 
	SoundBuffer::_vorbisRead, 
	SoundBuffer::_vorbisSeek, 
	SoundBuffer::_vorbisClose, 
	SoundBuffer::_vorbisTell 
};

///////////////////////////////////////

SoundBuffer::SoundBuffer( ResourceGroup* creator, const String& path ) :
Resource( creator, path ),
size(0),
mDuration( 0 )
{
	DEBUG_ASSERT( creator, "SoundBuffer needs a creator object" );
}

SoundBuffer::~SoundBuffer()
{

}

bool SoundBuffer::onLoad()
{
	DEBUG_ASSERT( isLoaded() == false, "The SoundBuffer is already loaded" );
	
	String ext = Utils::getFileExtension( filePath );
	
	DEBUG_ASSERT( ext == String( "ogg" ), "Sound file extension is not ogg" );
	
	_loadOggFromFile();
			
	return alGetError() != AL_NO_ERROR;
}


void SoundBuffer::onUnload(bool soft)
{
	DEBUG_ASSERT( isLoaded(), "SoundBuffer is not loaded" );

	/*if( !soft || isReloadable() )
	{
		alDeleteBuffers(1, &buffer);
		buffer = AL_NONE;
	}*/

	DEBUG_TODO;
}

////-------------------------------------////-------------------------------------////------------------------------------
//	STATIC VORBIS CALLBACK METHODS

size_t SoundBuffer::_vorbisRead( void* out, size_t size, size_t count, void* userdata )
{
	Stream* source = (Stream*)userdata;

	return source->read( (byte*)out, size * count );
}

int SoundBuffer::_vorbisSeek( void *userdata, ogg_int64_t offset, int whence )
{
	Stream* source = (Stream*)userdata;

	return source->seek( offset, whence );
}

int SoundBuffer::_vorbisClose( void *userdata )
{
	Stream* source = (Stream*)userdata;

	source->close();
	return 0;
}

long SoundBuffer::_vorbisTell( void *userdata )
{
	return ((Stream*)userdata)->getCurrentPosition();
}

bool SoundBuffer::Chunk::onLoad()
{
	loaded = false;

	//reopen the source
	Stream* source = pParent->mSource;
	if( !source->isOpen() )
		source->open();

	DEBUG_ASSERT( source->isReadable(), "The data source for the Ogg stream could not be open, or isn't readable" );

	int compressedSize = mEndPosition - mStartPosition;
	char* uncompressedData = (char*)malloc( MAX_SIZE );

	OggVorbis_File file;
	vorbis_info* info;
	ALenum format;
	int totalRead = 0;
	
	int error = ov_open_callbacks( source, &file, NULL, 0, VORBIS_CALLBACKS );
	
	DEBUG_ASSERT( error == 0, "Cannot load an ogg from the memory buffer" );
	
	info = ov_info( &file, -1 );
	
	int wordSize = 2;
	format = (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	
	int bitrate = info->rate;// * info->channels;
	int section = -1;

	//read all vorbis packets in the same buffer
	long read = 0;

	//seek to the start of the file segment
	error = ov_raw_seek( &file, mStartPosition );
	DEBUG_ASSERT( error == 0, "Cannot seek into file" );

	bool corrupt = false;
	do
	{
		read = ov_read( &file, uncompressedData + totalRead, MAX_SIZE - totalRead, 0, wordSize, 1, &section );

		if( read == OV_HOLE || read == OV_EBADLINK || read == OV_EINVAL )
			corrupt = true;

		else if( read == 0 )
			break;

		else
			totalRead += read;

		DEBUG_ASSERT( totalRead <= MAX_SIZE, "Total read bytes overflow the buffer" ); //this should always be true

	} while( !corrupt );

	ov_clear( &file );
	
	if( totalRead == 0 ) //no bytes were read!
		return false;
	
	alBufferData( alBuffer, format, uncompressedData, totalRead, bitrate );
	
	DEBUG_ASSERT( alGetError() == AL_NO_ERROR, "alBufferData error, cannot load a SoundBuffer" );
	
	free( uncompressedData );
	
	return loaded = alGetError() == AL_NO_ERROR;
}

void SoundBuffer::Chunk::onUnload( bool soft /* = false */ )
{

}

bool SoundBuffer::_loadOgg( Stream* source )
{
	DEBUG_ASSERT( source, "the data source cannot be null" );

	if( !source->open() )
	{
		DEBUG_ASSERT( mSource->isReadable(), "The data source for the Ogg stream could not be open, or isn't readable" );

		return false;
	}

	OggVorbis_File file;
	vorbis_info* info;
	ALenum format;
	int uncompressedSize, totalRead = 0;
	
	int error = ov_open_callbacks( source, &file, NULL, 0, VORBIS_CALLBACKS );
	
	DEBUG_ASSERT( error == 0, "Cannot load an ogg from the memory buffer" );
	
	info = ov_info( &file, -1 );
	
	int wordSize = 2;
	format = (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	
	int bitrate = info->rate;// * info->channels;
	int pcm = (int)ov_pcm_total( &file, -1 );
	int section = -1;

	uncompressedSize = pcm * wordSize * info->channels;

	mDuration = (float)pcm / (float)info->rate;

	int fileStart = 0;
	int fileEnd = -1;
	for( int i = 1; 1; ++i )
	{
		//find where in the file is the next chunk
		int pcmidx = min( pcm, i* Chunk::MAX_SIZE );
		error = ov_pcm_seek( &file, pcmidx );
		
		DEBUG_ASSERT( error != OV_ENOSEEK, "cannot seek" );
		DEBUG_ASSERT( error != OV_EINVAL, "invalid value" );
		DEBUG_ASSERT( error != OV_EREAD, "cannot read" );
		DEBUG_ASSERT( error != OV_EFAULT, "internal fault" );
		DEBUG_ASSERT( error != OV_EBADLINK, "?" );

		fileEnd = ov_raw_tell( &file );

		if( fileStart == fileEnd )
			break;

		//create it
		mChunks.add( new Chunk( this, fileStart, fileEnd ) );
		fileStart = fileEnd;
	}

	ov_clear( &file );

	//preload the first chunks
	for( int i = 0; i < SoundSource::QUEUE_SIZE && i < mChunks.size(); ++i )
		mChunks[i]->onLoad();

	if( !isStreaming() )
		mChunks[0]->get(); //get() it to avoid that it is unloaded by the sources

	return true;
}

bool SoundBuffer::_loadOggFromFile()
{
	mSource = Platform::getSingleton()->getFile( filePath );
	
	_loadOgg( mSource );

	return isLoaded();
}
