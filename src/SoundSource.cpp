#include "stdafx.h"

#include "SoundSource.h"
#include "SoundManager.h"
#include "Platform.h"

using namespace Dojo;

SoundSource::SoundSource( ALuint src ) :
buffer( NULL ),
source( src ),
pos(0,0),
positionChanged( true )
{
	_reset();
}

void SoundSource::_reset()
{
	state = SS_INITIALISING;
	
	pos = Vector::ZERO;
	positionChanged = true;
	buffer = NULL;
	mFrontChunk = mBackChunk = nullptr;
	mCurrentChunkID = 0;

	//set default parameters
	volume = 1.0f;
	pitch = 1.0f;
	looping = false;

	setAutoRemove(true);
}

SoundSource::~SoundSource()
{
	//the SoundSource takes ownership on the AL src
	alDeleteBuffers( 1, &source );
}

void SoundSource::setVolume( float v )
{	
	DEBUG_ASSERT( v >= 0, "Sound volumes cannot be negative" );
	
	volume = v * Platform::getSingleton()->getSoundManager()->getMasterVolume();
	
	if( source ) alSourcef (source, AL_GAIN, volume );		
}


float SoundSource::getVolume()
{
	float volume;
	alGetSourcef( source, AL_GAIN, &volume );
	return volume;
}

void SoundSource::play( float volume )
{
	//can the sound play?
	if( !isValid() && buffer && buffer->isLoaded() && Platform::getSingleton()->getSoundManager()->getMasterVolume() > 0 )
		return;

	if(state == SS_INITIALISING)
	{
		if(source && buffer )
		{
			CHECK_AL_ERROR;

			//set global parameters
			alSourcef (source, AL_REFERENCE_DISTANCE, 1.0f );

			int chunkNumber = buffer->getChunkNumber();

			mFrontChunk = buffer->getChunk( 0 );
			ALuint alBuffer = mFrontChunk->getOpenALBuffer();

			if( chunkNumber == 1 )  //non-streaming
			{
				alSourcei (source, AL_BUFFER, alBuffer );
				CHECK_AL_ERROR;	
			}
			else //use a queue
			{				
				alSourceQueueBuffers( source, 1, &alBuffer );
				mQueuedChunks = 1;
				CHECK_AL_ERROR;

				//start loading in the back buffer
				mBackChunk = buffer->getChunk( ++mCurrentChunkID, true );
			}
		}
		else
			state = SS_FINISHED;
	}

	if(state == SS_INITIALISING || state == SS_PAUSED)
	{
		setVolume( volume );
		setPitch( pitch );
		setLooping( looping );		
		
		//it can be moving, update pos
		if(positionChanged)
		{
			SoundManager::vectorToALfloat( pos , position);
			
			alSourcefv(source, AL_POSITION, position);
			
			positionChanged = false;
		}
		
		//actually play the sound
		alSourcePlay( source );

		alGetSourcefv(source, AL_POSITION, position);
			
		state = SS_PLAYING;	

		CHECK_AL_ERROR;
	}
}

void SoundSource::pause()
{
	if(state == SS_PLAYING)
	{
		state = SS_PAUSED;

		alSourcePause(source);
	}
}

void SoundSource::rewind()
{
	if(state != SS_FINISHED)
	{
		state = SS_INITIALISING;
		
		alSourceRewind(source);
	}
}


void SoundSource::_update()
{
	//it can be moving, update pos
	if(positionChanged)
	{
		SoundManager::vectorToALfloat( pos , position);
		
		alSourcefv(source, AL_POSITION, position);
		
		positionChanged = false;
	}
	
    //if streaming, check if buffers have been used and replenish the queue
	if( isStreaming() && mBackChunk )
	{
		//check if the backbuffer has finished loading and add it to the queue
		if( mQueuedChunks == 1 && mBackChunk->isLoaded()  )
		{
			ALuint b = mBackChunk->getOpenALBuffer();
			alSourceQueueBuffers( source, 1, &b );
			++mQueuedChunks;
			CHECK_AL_ERROR;
		}

		//check if the front buffer stopped streaming, move and pull a new backbuffer
		ALint processed;
		alGetSourcei( source, AL_BUFFERS_PROCESSED, &processed );

		if( processed )
		{
			//pop the old buffer
			ALuint b;
			alSourceUnqueueBuffers( source, 1, &b );
			--mQueuedChunks;
			CHECK_AL_ERROR;
			
			mFrontChunk->release();
			mFrontChunk = mBackChunk;

			//find the new buffer ID, loop if the source is looping, else go OOB and stop
			++mCurrentChunkID;
			if( looping && mCurrentChunkID >= buffer->getChunkNumber() )
				mCurrentChunkID = mCurrentChunkID % buffer->getChunkNumber();
			
			if( mCurrentChunkID < buffer->getChunkNumber() ) //not exhausted? start loading a new backbuffer
				mBackChunk = buffer->getChunk( mCurrentChunkID, true );

			else
				mBackChunk = nullptr;			
		}
	}
    
	alGetSourcei(source, AL_SOURCE_STATE, &playState);

	if( autoRemove && state == SS_PLAYING && playState == AL_STOPPED )
	{
		alSourcei( source, AL_BUFFER, AL_NONE ); //clear the buffer for source reusing - this ALSO works for queued buffers 
		
		//release all the used chunks
		if( mFrontChunk )
			mFrontChunk->release();

		if( mBackChunk )
			mBackChunk->release();

		state = SS_FINISHED;
	}
}