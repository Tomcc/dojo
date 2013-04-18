#include "stdafx.h"

#include "SoundManager.h"
#include "SoundSource.h"
#include "SoundBuffer.h"

#include "Platform.h"

#include "Utils.h"

using namespace Dojo;

const float SoundManager::m = 100;

///////////////////////////////////////

SoundManager::SoundManager() :
musicTrack( NULL ),
nextMusicTrack( NULL ),
fadeState( FS_NONE ),
musicVolume( 1 ),
masterVolume( 1 ),
currentFadeTime(0)
{		
	alGetError();

	// Initialization
	device = alcOpenDevice(NULL); // select the "preferred device"
	
	DEBUG_ASSERT( device, "Cannot open an OpenAL device" );

	context = alcCreateContext(device,NULL);
		
	DEBUG_ASSERT( context, "Cannot create an OpenAL context" );
		
	alcMakeContextCurrent(context);

	CHECK_AL_ERROR;

	//preload sounds
	DEBUG_ASSERT( NUM_SOURCES_MAX >= NUM_SOURCES_MIN, "Min source number cannot be > Max source number" );

	//create at least MIN sources, the rest will be lazy-loaded
	for( int i = 0; i < NUM_SOURCES_MIN; ++i )
	{
		ALuint src;
		alGenSources(1, &src );

		if( alGetError() == AL_NO_ERROR )
			idleSoundPool.add( new SoundSource( src ) );
		else
			break;
	}

	DEBUG_ASSERT( idleSoundPool.size() >= NUM_SOURCES_MIN, "OpenAL could not preload at least NUM_SOURCES_MIN sources" ); //ensure at least MIN sources have been built

	//dummy source to manage source shortage
	fakeSource = new SoundSource( 0 );

	setListenerPosition( Vector::ZERO );
	setListenerOrientation( Vector::UNIT_Z, Vector::UNIT_Y );

	CHECK_AL_ERROR;
}

SoundManager::~SoundManager()
{
	//trash sounds
	for( int i = 0; i < busySoundPool.size(); ++i )
		SAFE_DELETE( busySoundPool.at(i) );
	
	for( int i = 0; i < idleSoundPool.size(); ++i )
		SAFE_DELETE( idleSoundPool.at(i) );

	SAFE_DELETE( fakeSource );
}

SoundSource* SoundManager::getSoundSource( SoundSet* set, int i )
{
	DEBUG_ASSERT( set, "Cannot get a source for a null SoundSet" );

	//try to lazy-create a new source, if allowed
	if( idleSoundPool.isEmpty() && busySoundPool.size() < NUM_SOURCES_MAX )
	{
		ALuint src;
		alGenSources( 1, & src );
		if( alGetError() == AL_NO_ERROR )
			idleSoundPool.add( new SoundSource( src ) );
	}

	//is there a source now?
	if( !idleSoundPool.isEmpty() )
	{
		SoundSource* s = idleSoundPool.top();
		idleSoundPool.pop();
		busySoundPool.add(s);

		s->_setup( set->getBuffer( i ) );

		return s;
	}
	//failed, return mute source
	return fakeSource;
}

void SoundManager::playMusic( SoundSet* next, float trackFadeTime /* = 0 */ )
{
	DEBUG_ASSERT( next, "null music source passed" );

	//override music activation if the system sound is in use
	if( Platform::getSingleton()->isSystemSoundInUse() )
		return;

	nextMusicTrack = getSoundSource( next );

	halfFadeTime = trackFadeTime*0.5f;
	currentFadeTime = 0;
		
	fadeState = FS_FADE_OUT;
}

void SoundManager::setMasterVolume( float volume )
{	
	DEBUG_ASSERT( volume >= 0, "Volumes cannot be negative" );

	masterVolume = volume;

	//update volumes (masterVolume is used inside setVolume!)
	if( !nextMusicTrack && musicTrack )
		musicTrack->setVolume( musicVolume );

	//update all the existing sounds
	for( SoundSource* s : busySoundPool )
		s->setVolume( s->getVolume() );
}

void SoundManager::update( float dt )
{
	SoundSource* current;
	//sincronizza le sources con i nodes
	for( int i = 0; i < busySoundPool.size(); ++i)
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

void SoundManager::resumeMusic()
{
	//resume music, but only if the user didn't enable itunes meanwhile!
	if( musicTrack && !Platform::getSingleton()->isSystemSoundInUse() )
		musicTrack->play();
}
