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

	//set default parameters
	volume = 1.0f;
	pitch = 1.0f;
	looping = false;

	setAutoRemove(true);
	setFlushMemoryWhenRemoved(false);
}

SoundSource::~SoundSource()
{

}

void SoundSource::setVolume( float v )
{	
	DEBUG_ASSERT( v >= 0 );
	
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
	//il suono e' abilitato a suonare?
	if( !isValid() && buffer && buffer->isLoaded() && mgr->getMasterVolume() > 0 )
		return;

	if(state == SS_INITIALISING)
	{
		if(source && buffer )
		{
			//set global parameters
			alSourcef (source, AL_REFERENCE_DISTANCE,    1.0f );

            //TODO - request 3 chunks to the buffer
			alSourcei (source, AL_BUFFER,   buffer->_getOpenALBuffer()  );
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
	
    //TODO - stop if the buffer tells there will be no more chunks
    //pause if there are no more chunks but the buffer is still "open" 
    
	alGetSourcei(source, AL_SOURCE_STATE, &playState);

	if( autoRemove && !looping && state == SS_PLAYING && playState == AL_STOPPED )
	{
		state = SS_FINISHED;

		//unload the used buffer!
		if(flush)
			buffer->onUnload();
	}
}