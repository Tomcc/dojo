#include "stdafx.h"

#include "SoundManager.h"
#include "SoundSource.h"
#include "SoundBuffer.h"

#include "Platform.h"

#include "Utils.h"

using namespace Dojo;

const float SoundManager::m = 100;

const SoundManager::Easing SoundManager::LinearEasing = []( float t )
{
    return t;
};

void SoundManager::vectorToALfloat(const Vector& vector, ALfloat* ALpos) {
	DEBUG_ASSERT(ALpos, "null AL position vector");

	ALpos[0] = vector.x / m;
	ALpos[1] = vector.y / m;
	ALpos[2] = vector.z / m;
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

	//ensure at least MIN sources have been built
	DEBUG_ASSERT_INFO( 
		idleSoundPool.size() >= NUM_SOURCES_MIN, 
		"OpenAL could not preload the minimum sources number", String("NUM_SOURCES_MIN = ") + NUM_SOURCES_MIN ); 

	//dummy source to manage source shortage
	fakeSource = new SoundSource( 0 );

	setListenerTransform(Matrix(1));

	CHECK_AL_ERROR;
}


void SoundManager::clear() {
	for (int i = 0; i < busySoundPool.size(); ++i)
	{
		SoundSource* s = busySoundPool.top();
		busySoundPool.pop();
		s->stop();
		SAFE_DELETE(s);
	}

	musicTrack = NULL;
	fadeState = FS_NONE;
}


SoundManager::~SoundManager()
{
	//trash sounds
	for( int i = 0; i < busySoundPool.size(); ++i )
		SAFE_DELETE( busySoundPool.at(i) );
	
	for( int i = 0; i < idleSoundPool.size(); ++i )
		SAFE_DELETE( idleSoundPool.at(i) );

	SAFE_DELETE( fakeSource );

	alcCloseDevice( device );
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

void SoundManager::playMusic( SoundSet* next, float trackFadeTime /* = 0 */, const Easing& easing )
{
    //TODO use easing
    
	DEBUG_ASSERT( next, "null music source passed" );

	//override music activation if the system sound is in use
	if( Platform::singleton().isSystemSoundInUse() )
		return;

	nextMusicTrack = getSoundSource( next );

	halfFadeTime = trackFadeTime*0.5f;
	currentFadeTime = 0;
		
	fadeState = FS_FADE_OUT;
}

void SoundManager::setMasterVolume( float volume )
{	
	DEBUG_ASSERT( volume >= 0, "Volumes cannot be negative" );

	if (std::abs(masterVolume - volume) > 0.01f || volume == 0) //avoid doing this too often
	{
		masterVolume = volume;

		//update volumes (masterVolume is used inside setVolume!)
		if (!nextMusicTrack && musicTrack)
			musicTrack->setVolume(musicVolume);

		//update all the existing sounds
		for (SoundSource* s : busySoundPool)
			s->setVolume(s->getVolume());
	}
}

void SoundManager::update( float dt )
{
	SoundSource* current;
	//sincronizza le sources con i nodes
	for( int i = 0; i < busySoundPool.size(); ++i)
	{
		current = busySoundPool[i];
		current->_update(dt);

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
	if( musicTrack && !Platform::singleton().isSystemSoundInUse() )
		musicTrack->play();
}

void SoundManager::setListenerTransform(const Matrix& worldTransform)
{
	glm::vec4 pos(0.f, 0.f, 0.f, 1.f), up(0.f,1.f,0.f,0.f), forward(0.f,0.f,-1.f, 0.f);

	pos = worldTransform * pos;
	forward = worldTransform * forward;
	up = worldTransform * up;

	ALfloat orientation[6] = { 
		forward.x, forward.y, forward.z,
		up.x, up.y,	up.z
	};

	alListenerfv(AL_POSITION, glm::value_ptr(pos));
	alListenerfv(AL_VELOCITY, glm::value_ptr(pos - lastListenerPos));
	alListenerfv(AL_ORIENTATION, orientation);

	lastListenerPos = pos;
}

SoundSource* SoundManager::getSoundSource(const Vector& pos, SoundSet* set) {
	DEBUG_ASSERT(set, "Getting a Source for a NULL sound");

	SoundSource* s = getSoundSource(set);
	s->setPosition(pos);

	return s;
}

SoundSource* SoundManager::playSound(SoundSet* set, float volume /*= 1.0f*/) {
	DEBUG_ASSERT(set, "Playing a NULL sound");

	SoundSource* s = getSoundSource(set);
	s->play(volume);
	return s;
}

SoundSource* SoundManager::playSound(const Vector& pos, SoundSet* set, float volume /*= 1.0f */) {
	DEBUG_ASSERT(set, "Playing a NULL sound");

	SoundSource* s = getSoundSource(pos, set);
	s->play(volume);
	return s;
}

void SoundManager::pauseMusic() {
	DEBUG_ASSERT(isMusicPlaying(), "pauseMusic: music is not playing");

	musicTrack->pause();
}

void SoundManager::stopMusic(float stopFadeTime /*= 0*/, const Easing& fadeEasing /*= LinearEasing */) {
	//TODO use easing
	DEBUG_MESSAGE("Music fading out in " + String(stopFadeTime) + " s");

	fadeState = FS_FADE_OUT;
	nextMusicTrack = NULL;

	halfFadeTime = stopFadeTime;
	currentFadeTime = 0;
}

void SoundManager::setMusicVolume(float volume) {
	DEBUG_ASSERT(volume >= 0, "setMusicVolume: volume is negative");

	musicVolume = volume;

	if (!nextMusicTrack && musicTrack)
		musicTrack->setVolume(musicVolume);
}

void SoundManager::pauseAll() {
	for (SoundSource* s : busySoundPool)
	{
		if (s != musicTrack)
			s->pause();
	}
}

void SoundManager::resumeAll() {
	for (SoundSource* s : busySoundPool)
	{
		if (s != musicTrack)
			s->play();
	}
}

void SoundManager::stopAllSounds() {
	for (SoundSource* s : busySoundPool)
	{
		if (s != musicTrack)
			s->stop();
	}
}
