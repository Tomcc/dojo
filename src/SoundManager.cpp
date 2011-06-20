#include "stdafx.h"

#include "SoundManager.h"
#include "SoundSource.h"
#include "SoundBuffer.h"

#include "Platform.h"

#include "Utils.h"

using namespace Dojo;

const float SoundManager::m = 100;

bool SoundManager::alCheckError()
{
	bool foundError = false;
	ALenum error;

	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		foundError = true;
	}

	return foundError;
}

///////////////////////////////////////

SoundManager::SoundManager() :
musicTrack( NULL ),
nextMusicTrack( NULL ),
fadeState( FS_NONE ),
musicVolume( 1 ),
masterVolume( 1 ),
currentFadeTime(0)
{		
	// Initialization
	device = alcOpenDevice(NULL); // select the "preferred device"
	
	DEBUG_ASSERT( device );
	
	int error = alCheckError();
	
	if (device) 
	{
		context = alcCreateContext(device,NULL);
		
		error = alCheckError();
		
		alcMakeContextCurrent(context);
	}
    // Clear Error Code
    alCheckError();

	alGenSources( MAX_SOURCES, sources );

	//preload sounds
	for( unsigned int i = 0; i < MAX_SOURCES; ++i )
		idleSoundPool.add( new SoundSource( this, sources[i] ) );

	//dummy source to manage source shortage
	fakeSource = new SoundSource( this, NULL );

	setListenerPosition( Vector::ZERO );
	setListenerOrientation( 0,0,1,0,1,0 );
}

SoundManager::~SoundManager()
{
	//trash sounds
	for( unsigned int i = 0; i < busySoundPool.size(); ++i )
		delete busySoundPool.at(i);
	
	for( unsigned int i = 0; i < idleSoundPool.size(); ++i )
		delete idleSoundPool.at(i);

	delete fakeSource;

	alDeleteSources( MAX_SOURCES, sources );
}

void SoundManager::playMusic( SoundSet* next, float trackFadeTime /* = 0 */ )
{
	DEBUG_ASSERT( next );

	//override music activation if the system sound is in use
	if( isMusicFading() || Platform::getSingleton()->isSystemSoundInUse() )
		return;

	nextMusicTrack = getSoundSource( next );

	halfFadeTime = trackFadeTime*0.5f;
	currentFadeTime = 0;
		
	fadeState = FS_FADE_OUT;
}

void SoundManager::update( float dt )
{
	SoundSource* current;
	//sincronizza le sources con i nodes
	for( unsigned int i = 0; i < busySoundPool.size(); ++i)
	{
		current = busySoundPool[i];
		current->_update();

		//resetta i suoni finiti
		if( current->_isWaitingForDelete() )
		{
			busySoundPool.remove( i );
			idleSoundPool.add( current );

			current->_reset();

			--i;
		}
	}
	
	//fai il fade
	if( fadeState == FS_FADE_OUT ) //abbassa il volume della track corrente
	{
		if( musicTrack && currentFadeTime < halfFadeTime )
			musicTrack->setVolume( musicVolume * ( 1.f - currentFadeTime/halfFadeTime) );
		else //scambia le tracks e fai partire la prossima
		{
			if( musicTrack )
				musicTrack->stop();

			musicTrack = nextMusicTrack;

			//parte il fade in
			if( musicTrack )
			{
				nextMusicTrack = NULL;

				musicTrack->play(0);
				musicTrack->setAutoRemove( false );

				musicTrack->setLooping( true );

				fadeState = FS_FADE_IN;
				currentFadeTime = 0;
			}
			else
				fadeState = FS_NONE;
		}
		
		currentFadeTime += dt;
	}
	else if( fadeState == FS_FADE_IN )  //alza il volume della track successiva
	{
		if( currentFadeTime < halfFadeTime )
			musicTrack->setVolume( musicVolume * (currentFadeTime/halfFadeTime) );

		else  //finisci
		{
			currentFadeTime = 0;
			
			//force volume at max
			musicTrack->setVolume( musicVolume );
			
			fadeState = FS_NONE;
		}
		
		currentFadeTime += dt;
	}
}

