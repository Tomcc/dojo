#ifndef SOUNDMANAGER_H_
#define SOUNDMANAGER_H_

#include "dojo_common_header.h"

#include "Array.h"
#include "Vector.h"

#include "SoundBuffer.h"
#include "SoundSet.h"
#include "SoundSource.h"

#define NUM_SOURCES_MIN 16
#define NUM_SOURCES_MAX 256

namespace Dojo {

		class SoundListener;
		class SoundSource;

		///Dojo's audio system, based on OpenAL
		class SoundManager 
		{
		public:
			typedef Array< SoundSource* > SoundList;
            
            typedef std::function< float(float) > Easing;
            
            static const Easing LinearEasing;
			static const float m;

			inline static void vectorToALfloat(const Vector& vector, ALfloat* ALpos )
			{
				DEBUG_ASSERT( ALpos, "null AL position vector" );
				
				ALpos[0] = vector.x/m;
				ALpos[1] = vector.y/m;
				ALpos[2] = vector.z/m;
			}

			SoundManager();

			~SoundManager();
			
			///clear() destroys the sound pool - use wisely!
			void clear()
			{
				for( int i = 0; i < busySoundPool.size(); ++i )
				{
					SoundSource* s = busySoundPool.top();
					busySoundPool.pop();
					s->stop();
					SAFE_DELETE( s );
				}
				
				musicTrack = NULL;
				fadeState = FS_NONE;
			}

			///Returns a sound source ready to play a new sound
			SoundSource* getSoundSource( SoundSet* set, int i = -1 );

			///Returns a sound source ready to play a new sound, with the position already set
			inline SoundSource* getSoundSource( const Vector& pos, SoundSet* set )
			{
				DEBUG_ASSERT( set, "Getting a Source for a NULL sound" );
				
				SoundSource* s = getSoundSource( set );
				s->setPosition( pos );

				return s;
			}

			///Plays the given set without spatial positioning
			inline SoundSource* playSound( SoundSet* set )
			{
				DEBUG_ASSERT( set, "Playing a NULL sound" );
				 
				SoundSource* s = getSoundSource( set );
				s->play();
				return s;
			}

			///Plays the given set at pos
			inline SoundSource* playSound( const Vector& pos, SoundSet* set )
			{
				DEBUG_ASSERT( set, "Playing a NULL sound" );
				
				SoundSource* s = getSoundSource( pos, set );
				s->play();
				return s;
			}
						
			///Starts a new sound using it as background music
			/**
			\param trackFadeTime the duration of the intro fade-in
			*/
			void playMusic( SoundSet* music, float trackFadeTime = 0, const Easing& fadeEasing = LinearEasing );
			
			inline void pauseMusic()
			{
                DEBUG_ASSERT( isMusicPlaying(), "pauseMusic: music is not playing" );
                
				musicTrack->pause();
			}
			
			void resumeMusic();

			///stops the music, with an optional fade-out
			inline void stopMusic( float stopFadeTime = 0, const Easing& fadeEasing = LinearEasing )
			{
                //TODO use easing
                DEBUG_MESSAGE( "Music fading out in " << stopFadeTime << " s" );
					
                fadeState = FS_FADE_OUT;
                nextMusicTrack = NULL;

                halfFadeTime = stopFadeTime;
                currentFadeTime = 0;
			}

			inline void setMusicVolume( float volume )			
			{	
				DEBUG_ASSERT( volume >= 0, "setMusicVolume: volume is negative" );
				
				musicVolume = volume;	

				if( !nextMusicTrack && musicTrack )
					musicTrack->setVolume( musicVolume );
			}
			
			void setMasterVolume( float volume );
			
			inline float getMasterVolume()				{	return masterVolume;		}
			inline float getMusicVolume()				{	return musicVolume;			}
			
			inline SoundSource* getMusicTrack()
			{
				return musicTrack;
			}
			
			///pauses all the active SoundSources (excluding the background music!)
			void pauseAll()
			{
				for( SoundSource* s : busySoundPool )
				{
					if( s != musicTrack )
						s->pause();
				}
			}
			///resumes all the active SoundSources (excluding the background music!)
			void resumeAll()
			{
				for( SoundSource* s : busySoundPool )
				{
					if( s != musicTrack )
						s->play();
				}
			}
			
			///stops all the active SoundSources (excluding the background music!)
			void stopAll()
			{
				for( SoundSource* s : busySoundPool )
				{
					if( s != musicTrack )
						s->stop();
				}
			}
			
			///true if the music is fading
			inline bool isMusicFading()		{	return fadeState != FS_NONE;	}
            ///is the music already playing?
            inline bool isMusicPlaying()    {   return musicTrack != NULL;      }
			
			///sets the openAL Listener's position
			inline void setListenerPosition( const Vector& pos )
			{				
				vectorToALfloat(pos , listenerPos );
				
				alListenerfv(AL_POSITION, listenerPos);
			}
			///sets the openAL Listener's orientation
			inline void setListenerOrientation( const Vector& forward, const Vector& up )
			{				
				orientation[0] = forward.x;
				orientation[1] = forward.y;
				orientation[2] = forward.z;
				orientation[3] = up.x;
				orientation[4] = up.y;
				orientation[5] = up.z;
				
				alListenerfv(AL_ORIENTATION, orientation);
			}

			void update( float dt );

		protected:

			enum FadeState
			{
				FS_NONE,
				FS_FADE_IN,
				FS_FADE_OUT
			};

			ALCcontext *context;
			ALCdevice *device;

			ALfloat listenerPos[3];					
			ALfloat orientation[6];

			//pool di suoni
			SoundList idleSoundPool;
			SoundList busySoundPool;

			SoundSource* fakeSource;

			SoundSource *musicTrack, *nextMusicTrack;
			float halfFadeTime;
			float currentFadeTime;
			FadeState fadeState;

			float musicVolume;
			float masterVolume;

			void _createMainBundleBuffers();		
			
		};
}

#endif
