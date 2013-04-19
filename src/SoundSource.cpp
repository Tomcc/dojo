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
	mCurrentChunk = 0;

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
			if( chunkNumber == 1 ) //immediate sound
			{
				CHECK_AL_ERROR;

				mCurrentChunk = buffer->getChunk( 0 );
				ALuint buf = buffer->getChunk( 0 )->getOpenALBuffer();
				alSourcei (source, AL_BUFFER, buf );

				CHECK_AL_ERROR;	
			}
			else
			{
				DEBUG_ASSERT( mChunkQueue.empty(), "The queue should have been emptied before reusing the queue" );

				//try to load at least BUFFER_QUEUE sounds
				for( mCurrentChunkID = 0; mCurrentChunkID < chunkNumber && mCurrentChunkID < QUEUE_SIZE; ++mCurrentChunkID )
				{
					//add to the queue
					auto chunk = buffer->getChunk( mCurrentChunkID );
					ALuint buf = chunk->getOpenALBuffer();
					alSourceQueueBuffers( source, 1, &buf );
					mChunkQueue.push( chunk );

					CHECK_AL_ERROR;
				}
				--mCurrentChunkID;

				CHECK_AL_ERROR;
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
	if( isStreaming() )
	{
		ALint processed;
		alGetSourcei( source, AL_BUFFERS_PROCESSED, &processed );

		if( processed )
		{
			//pop the old buffer
			ALuint b;
			alSourceUnqueueBuffers( source, 1, &b );

			auto chunk = mChunkQueue.front();
			mChunkQueue.pop();
			chunk->release();

			//find the new buffer ID, loop if the source is looping, else go OOB
			++mCurrentChunkID;
			if( looping && mCurrentChunkID >= buffer->getChunkNumber() )
				mCurrentChunkID = mCurrentChunkID % buffer->getChunkNumber();
			
			if( mCurrentChunkID < buffer->getChunkNumber() ) //exhausted?
			{
				//queue the new buffer
				chunk = buffer->getChunk( mCurrentChunkID );
				mChunkQueue.push( chunk );
				b = chunk->getOpenALBuffer();

				alSourceQueueBuffers( source, 1, &b );
			}
			
			CHECK_AL_ERROR;
		}
	}
    
	alGetSourcei(source, AL_SOURCE_STATE, &playState);

	if( autoRemove && state == SS_PLAYING && playState == AL_STOPPED )
	{
		alSourcei( source, AL_BUFFER, AL_NONE ); //clear the buffer for source reusing - this ALSO works for queued buffers 

		ALint n;
		alGetSourcei( source, AL_BUFFERS_PROCESSED, &n );
		DEBUG_ASSERT( n == 0, "This OpenAL implementation fails in cleaning the sources with AL_NONE" );

		CHECK_AL_ERROR;

		//release all the used chunks
		if( !isStreaming() ) //nonstreaming
			mCurrentChunk->release();
	
		else
		{
			while( !mChunkQueue.empty() )
			{
				mChunkQueue.front()->release();
				mChunkQueue.pop();
			}
		}

		state = SS_FINISHED;
	}
}