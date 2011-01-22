#include "SoundManager.h"
#include "SoundSource.h"
#include "SoundBuffer.h"

#include "Utils.h"

#include <AudioToolbox/AudioToolbox.h>

using namespace Dojo;

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
	//set audio context on the iphone device
	AudioSessionInitialize ( NULL, NULL, 
							NULL, // may want interruption callback here
							NULL );
	
	//if using mp3 playback, it is needed to exclude other applications' sounds
#ifdef HARDWARE_SOUND
	UInt32 sessionCategory = kAudioSessionCategory_SoloAmbientSound;    // 1
#else
	UInt32 sessionCategory = kAudioSessionCategory_AmbientSound;
#endif
	AudioSessionSetProperty (kAudioSessionProperty_AudioCategory, sizeof (sessionCategory), &sessionCategory);
	AudioSessionSetActive (true);	
	
	// Initialization
	device = alcOpenDevice(NULL); // select the "preferred device"
	
	DEBUG_ASSERT( device );
	
	int error = alCheckError();
	
	if (device) 
	{
		context=alcCreateContext(device,NULL);
		
		error = alCheckError();
		
		alcMakeContextCurrent(context);
	}
    // Clear Error Code
    alCheckError();

	//preload sounds
	for( unsigned int i = 0; i < maxSources; ++i )
		idleSoundPool.addElement( new SoundSource( this ) );

	//crea il suono inutile
	dummySound = new SoundDummy( this );
		
	//look for all the sounds in the bundle
	_createMainBundleBuffers();
}

SoundManager::~SoundManager()
{
	//trash sounds
	for( unsigned int i = 0; i < busySoundPool.size(); ++i )
		delete busySoundPool.at(i);
	
	for( unsigned int i = 0; i < idleSoundPool.size(); ++i )
		delete idleSoundPool.at(i);
}

void SoundManager::_createMainBundleBuffers()
{
	//ask all the sound files to the main bundle
	NSArray* paths = [[NSBundle mainBundle] pathsForResourcesOfType: @"caf" inDirectory: @"data" ];
	NSString* NSpath;
	std::string name, lastName, path;
	SoundSet* currentSet = NULL;
	
	for( uint i = 0; i < [paths count]; ++i )
	{
		NSpath = (NSString*)[paths objectAtIndex:i];
		
		path = Utils::toSTDString( NSpath ); 
		name = Utils::getFileName( path );
		
		if( !Utils::areStringsNearInSequence( lastName, name ) )
		{
			std::string setPrefix = Utils::removeTag( name );
			
			//create a new set
			currentSet = new SoundSet( setPrefix );
			
			soundDataMap[ setPrefix ] = currentSet;
		}
			
		//create and load a new buffer
		SoundBuffer* b = new SoundBuffer( this, path );
		b->load();
		
		//TODO: Multiple Sound-in-set Support
		currentSet->addBuffer( b );
		
		lastName = name;
	}
	
}

bool SoundManager::isSystemSoundInUse()
{
	UInt32 otherAudioIsPlaying;
	UInt32 size = sizeof(otherAudioIsPlaying);
	AudioSessionGetProperty(kAudioSessionProperty_OtherAudioIsPlaying, &size, &otherAudioIsPlaying);
	
	return otherAudioIsPlaying;
}


void SoundManager::playMusic( const std::string& name, float trackFadeTime /* = 0 */ )
{
	//override music activation if the system sound is in use
	if( isMusicFading() || isSystemSoundInUse() )
		return;

	SoundSource* next = getSoundSource(name);

	if( next )
	{
		nextMusicTrack = next;

		halfFadeTime = trackFadeTime*0.5f;
		currentFadeTime = 0;
		
		fadeState = FS_FADE_OUT;
	}
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
			busySoundPool.popElement( i );
			idleSoundPool.addElement( current );

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

