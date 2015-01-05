#include "stdafx.h"

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "SoundBuffer.h"

#include "SoundManager.h"
#include "MemoryInputStream.h"
#include "FileStream.h"
#include "Utils.h"
#include "Platform.h"
#include "BackgroundQueue.h"

using namespace Dojo;

//STATIC CALLBACKS
size_t _vorbisRead(void* out, size_t size, size_t count, void* userdata)
{
	Stream* source = (Stream*)userdata;

	return source->read((byte*)out, size * count);
}

int _vorbisSeek(void *userdata, ogg_int64_t offset, int whence)
{
	Stream* source = (Stream*)userdata;

	return source->seek((long)offset, whence);
}

int _vorbisClose(void *userdata)
{
	Stream* source = (Stream*)userdata;

	source->close();
	return 0;
}

long _vorbisTell(void *userdata)
{
	return ((Stream*)userdata)->getCurrentPosition();
}


#define OGG_ENDIAN 0

//store the callbacks
ov_callbacks VORBIS_CALLBACKS = 
{ 
	_vorbisRead, 
	_vorbisSeek, 
	_vorbisClose, 
	_vorbisTell 
};


SoundBuffer::Chunk::Chunk(SoundBuffer* parent, long streamStartPosition, long uncompressedSize) :
size(0),
alBuffer(AL_NONE),
references(0),
pParent(parent),
mStartPosition(streamStartPosition),
mUncompressedSize(uncompressedSize) {
	DEBUG_ASSERT(parent, "invalid parent");
	DEBUG_ASSERT(streamStartPosition >= 0, "invalid starting position");
	DEBUG_ASSERT(uncompressedSize > 0, "invalid PCM span size");
}

SoundBuffer::Chunk::~Chunk() {
	alDeleteBuffers(1, &alBuffer);
}

void SoundBuffer::Chunk::getAsync() {
	if (references++ == 0 && !isLoaded()) //load it when referenced the first time
		loadAsync();
}

void SoundBuffer::Chunk::get() {
	if (references++ == 0 && !isLoaded()) //load it when referenced the first time
		onLoad();
}

void SoundBuffer::Chunk::release() {
	DEBUG_ASSERT(references > 0, "References should never be less than 0");

	if (--references == 0) //unload it when dereferenced last time
		onUnload();
}

ALuint SoundBuffer::Chunk::getOpenALBuffer() {
	DEBUG_ASSERT(isLoaded(), "This buffer is not loaded and has no AL buffer");

	return alBuffer;
}


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
			
	return CHECK_AL_ERROR;
}


void SoundBuffer::onUnload(bool soft)
{
	DEBUG_ASSERT( isLoaded(), "SoundBuffer is not loaded" );

	//just push the event to all its chunks
	for( auto chunk : mChunks )
	{
		if( !isStreaming() ) //non-streaming buffers own all of their chunk (to avoid them being released each time)
			chunk->release();

		if( chunk->isLoaded() )
			chunk->onUnload( soft );
	}
}

bool SoundBuffer::Chunk::onLoad()
{
	DEBUG_ASSERT( !isLoaded(), "The Chunk is already loaded" );

	alGenBuffers( 1, &alBuffer ); //gen the buffer if it didn't exist

	CHECK_AL_ERROR;
    
    //copy the source to avoid side-effects
    Unique< Stream > source( pParent->mSource->copy() );
    
    source->open();

	DEBUG_ASSERT( source->isReadable(), "The data source for the Ogg stream could not be open, or isn't readable" );

	char* uncompressedData = (char*)malloc( mUncompressedSize );

	OggVorbis_File file;
	vorbis_info* info;
	ALenum format;
	int totalRead = 0;

	int error = ov_open_callbacks( source.get(), &file, NULL, 0, VORBIS_CALLBACKS );

	DEBUG_ASSERT( error == 0, "Cannot load an ogg from the memory buffer" );

	info = ov_info( &file, -1 );

	int wordSize = 2;
	format = (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

	int bitrate = info->rate * 10; //wtf, why * 10 //HACK

	//read all vorbis packets in the same buffer
	long read = 0;

	//seek to the start of the file segment
	error = ov_raw_seek( &file, mStartPosition );
	DEBUG_ASSERT( error == 0, "Cannot seek into file" );

	bool corrupt = false;
	do
	{
		int section = -1;
		read = ov_read( &file, uncompressedData + totalRead, mUncompressedSize - totalRead, 0, wordSize, 1, &section );

		if( read == OV_HOLE || read == OV_EBADLINK || read == OV_EINVAL )
			corrupt = true;

		else if( read == 0 )
			break;

		else
			totalRead += read;

		DEBUG_ASSERT( totalRead <= mUncompressedSize, "Total read bytes overflow the buffer" ); //this should always be true

	} while( !corrupt && totalRead < mUncompressedSize );

	ov_clear( &file );

	DEBUG_ASSERT( !corrupt, "an ogg vorbis stream was corrupt and could not be read" );
	DEBUG_ASSERT( totalRead > 0, "no data was read from the stream" );

	alBufferData( alBuffer, format, uncompressedData, totalRead, bitrate );

	loaded = CHECK_AL_ERROR;

	free( uncompressedData );

	return loaded;
}

void SoundBuffer::Chunk::loadAsync()
{
	DEBUG_ASSERT( !isLoaded(), "The Chunk is already loaded" );

	++references; //grab a reference and release to be sure that the chunk is not destroyed while loading

	//async load
	Platform::singleton().getBackgroundQueue()->queueTask([&]()
	{
		onLoad();

		std::this_thread::sleep_for(std::chrono::milliseconds(20)); //HACK
	},
	[&]() //then,
	{
		release();
	});
}

void SoundBuffer::Chunk::onUnload( bool soft /* = false */ )
{
	//either unload if forced, or if the parent is reloadable (loaded from file or persistent source)
	if( !soft || pParent->isReloadable() )
	{
		DEBUG_ASSERT( isLoaded(), "Tried to unload an unloaded Chunk" );
        DEBUG_ASSERT( alBuffer != AL_NONE, "tried to delete an invalid alBuffer" );
		DEBUG_ASSERT( references == 0, "Tried to unload a Chunk that is still in use" );

		alDeleteBuffers( 1, &alBuffer );

		CHECK_AL_ERROR;

		alBuffer = 0;
		loaded = false;
	}
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
	ogg_int64_t uncompressedSize;
	
	int error = ov_open_callbacks( source, &file, NULL, 0, VORBIS_CALLBACKS );
	
	DEBUG_ASSERT( error == 0, "Cannot load an ogg from the memory buffer" );
	
	info = ov_info( &file, -1 );
	
	int wordSize = 2;
	format = (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	
	ogg_int64_t totalPCM = ov_pcm_total( &file, -1 );

	uncompressedSize = totalPCM * wordSize * info->channels;

	//find the number of chunks that we want
	int chunkN = 1;
	//HACK for( ; uncompressedSize / chunkN > Chunk::MAX_SIZE; chunkN++ );
	ogg_int64_t chunkPCM = totalPCM / chunkN;

	mDuration = (float)totalPCM / (float)info->rate;

	ogg_int64_t fileStart = 0, fileEnd = -1;
	ogg_int64_t pcmStart = 0, pcmEnd = -1;

	//load all the needed chunks
	while(1)
	{
		pcmEnd = min( totalPCM, pcmStart + chunkPCM );

		ov_pcm_seek_page( &file, pcmEnd );
		fileEnd = ov_raw_tell( &file );

		if( fileStart == fileEnd ) //check if EOF
			break;

		if( pcmEnd == pcmStart ) //buffer is too small, let's assume that there is just one page
			pcmEnd = totalPCM; 

		ogg_int64_t byteSize = (pcmEnd-pcmStart) * wordSize * info->channels;
		mChunks.add( new Chunk( this, (long)fileStart, (long)byteSize ) );

		pcmStart = pcmEnd;
		fileStart = fileEnd;
	}

	ov_clear( &file );

	if( !isStreaming() )
		mChunks[0]->get(); //get() it to avoid that it is unloaded by the sources, and load synchronously

	return true;
}

bool SoundBuffer::_loadOggFromFile()
{
	mFile = Platform::singleton().getFile( filePath );
	mSource = mFile.get();
	
	_loadOgg( mSource );

	return isLoaded();
}

SoundBuffer::Chunk* SoundBuffer::getChunk(int n, bool loadAsync /*= false */) {
	DEBUG_ASSERT(n >= 0 && n < mChunks.size(), "The requested chunk is out of bounds");

	if (loadAsync)
		mChunks[n]->getAsync();
	else
		mChunks[n]->get();

	return mChunks[n];
}
