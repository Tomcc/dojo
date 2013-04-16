#include "stdafx.h"

#include "SoundSource.h"
#include "SoundManager.h"

using namespace Dojo;

SoundSource::SoundSource( SoundManager* manager, ALuint src ) :
buffer( NULL ),
source( src ),
mgr( manager ),
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
	
	volume = v * mgr->getMasterVolume();
	
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
	if( !isValid() && buffer && buffer->isLoaded() && mgr->getMasterVolume() > 0 )
		return;

	if(state == SS_INITIALISING)
	{
		if(source && buffer )
		{
			//set global parameters
			alSourcef (source, AL_REFERENCE_DISTANCE, 1.0f );

			int chunkNumber = buffer->getChunkNumber();
			if( chunkNumber == 1 ) //immediate sound
			{
				mCurrentChunk = buffer->getChunk( 0 );

				alSourcei (source, AL_BUFFER, buffer->getChunk( 0 )->getOpenALBuffer() );
			}
			else
			{				
				DEBUG_ASSERT( mChunkQueue.empty(), "The queue should have been emptied before reusing the queue" );

				//try to load at least BUFFER_QUEUE sounds
				for( mCurrentChunkID = 0; mCurrentChunkID < chunkNumber && mCurrentChunkID < QUEUE_SIZE; ++mCurrentChunkID )
				{
					//add to the queue
					auto chunk = buffer->getChunk( mCurrentChunkID );
					chunk->get();
					ALuint buf = chunk->getOpenALBuffer();
					alSourceQueueBuffers( source, 1, &buf );

					mChunkQueue.push( chunk );
				}
				--mCurrentChunkID;
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

			//add a new buffer
			++mCurrentChunkID;
			if( looping && mCurrentChunkID >= buffer->getChunkNumber() )
				mCurrentChunkID = mCurrentChunkID % buffer->getChunkNumber();
			
			if( mCurrentChunkID < buffer->getChunkNumber() ) //exhausted?
			{
				chunk = buffer->getChunk( mCurrentChunkID );
				chunk->get();
				mChunkQueue.push( chunk );
			}
		}
	}
    
	alGetSourcei(source, AL_SOURCE_STATE, &playState);

	if( autoRemove && !looping && state == SS_PLAYING && playState == AL_STOPPED )
	{
		//take care of the chunks
		if( mCurrentChunk ) //nonstreaming
			mCurrentChunk->release();
		else
		{
			DEBUG_TODO; //release the exhausted buffers in the queue
		}

		state = SS_FINISHED;
	}
}