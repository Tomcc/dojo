#ifndef SOUNDMANAGER_H_
#define SOUNDMANAGER_H_

#include "dojo_common_header.h"

#include "Array.h"
#include "Vector.h"

#include "SoundBuffer.h"
#include "SoundSet.h"
#include "SoundSource.h"

#define MAX_SOURCES 32

namespace Dojo {

		class SoundListener;
		class SoundSource;

		///Classe che gestisce il sistema Audio di Dojo.
		class SoundManager 
		{
		public:
			typedef Array< SoundSource* > SoundList;

			static const float m;

			///metodo statico per convertire la classe vector in vettore C-style.
			inline static void vectorToALfloat(const Vector& vector, ALfloat* ALpos )
			{
				DEBUG_ASSERT( ALpos );
				
				*ALpos++ = vector.x/m;
				*ALpos++ = vector.y/m;
				*ALpos = 0;
			}

			static bool alCheckError();

			///Costruttore
			SoundManager();
			~SoundManager();
			
			///clear() destroys the sound pool - use wisely!
			void clear()
			{
				for( uint i = 0; i < busySoundPool.size(); ++i )
				{
					SoundSource* s = busySoundPool.top();
					busySoundPool.pop();
					s->stop();
					SAFE_DELETE( s );
				}
				
				musicTrack = NULL;
				fadeState = FS_NONE;
			}

			///restituisce una fonte sonora con il suono dato
			inline SoundSource* getSoundSource( SoundSet* set, int i = -1 )
			{
				DEBUG_ASSERT( set );

				if( idleSoundPool.size() > 0 && set )
				{
					SoundSource* s = idleSoundPool.top();
					idleSoundPool.pop();
					busySoundPool.add(s);

					s->_setup( set->getBuffer( i ) );

					return s;
				}
				return fakeSource;
			}

			inline SoundSource* getSoundSource( const Vector& pos, SoundSet* set )
			{
				DEBUG_ASSERT( set );
				
				SoundSource* s = getSoundSource( set );
				s->setPosition( pos );

				return s;
			}

			inline SoundSource* playSound( SoundSet* set )
			{
				DEBUG_ASSERT( set );
				
				SoundSource* s = getSoundSource( set );
				s->play();
				return s;
			}

			inline SoundSource* playSound( const Vector& pos, SoundSet* set )
			{
				DEBUG_ASSERT( set );
				
				SoundSource* s = getSoundSource( pos, set );
				s->play();
				return s;
			}
						
			///setta la musica facendo un fade lineare in fadeTime rispetto alla track precedente
			void playMusic( SoundSet* music, float trackFadeTime = 0 );
			
			inline void pauseMusic()
			{
				if( musicTrack )
					musicTrack->pause();
			}
			
			void resumeMusic();

			///interrompe la traccia corrente con un certo fade
			inline void stopMusic( float stopFadeTime = 0 )
			{
				if( musicTrack && !isMusicFading() )
				{
					DEBUG_MESSAGE( "Music fading out in " << stopFadeTime << " s" );
					
					fadeState = FS_FADE_OUT;
					nextMusicTrack = NULL;

					halfFadeTime = stopFadeTime;
					currentFadeTime = 0;
				}
			}

			inline void setMusicVolume( float volume )			
			{	
				DEBUG_ASSERT( volume >= 0 );
				
				musicVolume = volume;	

				if( !nextMusicTrack && musicTrack )
					musicTrack->setVolume( musicVolume );
			}
			
			inline void setMasterVolume( float volume )	
			{	
				DEBUG_ASSERT( volume >= 0 );
				
				masterVolume = volume;		
			}
			
			inline float getMasterVolume()				{	return masterVolume;		}
			inline float getMusicVolume()				{	return musicVolume;			}
			
			///metodo utile per mettere in pausa tutte le sources attive
			void pauseAll()
			{
				SoundSource* s;
				for( uint i = 0; i < busySoundPool.size(); ++i )
				{
					s = busySoundPool.at(i);
					if( s != musicTrack )
						busySoundPool.at(i)->pause();
				}
			}
			
			void resumeAll()
			{
				SoundSource* s;
				for( uint i = 0; i < busySoundPool.size(); ++i )
				{
					s = busySoundPool.at(i);
					if( s != musicTrack )
						busySoundPool.at(i)->play();
				}
			}
			
			///sopts all playing sounds
			void stopAll()
			{
				SoundSource* s;
				for( uint i = 0; i < busySoundPool.size(); ++i )
				{
					s = busySoundPool.at(i);
					if( s != musicTrack )
						busySoundPool.at(i)->stop();
				}
			}
			
			//c'e' gia' un fade in corso?
			inline bool isMusicFading()		{	return fadeState != FS_NONE;	}
			
			inline void setListenerPosition( const Vector& pos )
			{				
				vectorToALfloat(pos , listenerPos );
				
				alListenerfv(AL_POSITION, listenerPos);
			}
			
			inline void setListenerOrientation( float fx, float fy, float fz, float ux, float uy, float uz )
			{				
				orientation[0] = fx;
				orientation[1] = fy;
				orientation[2] = fz;
				orientation[3] = ux;
				orientation[4] = uy;
				orientation[5] = uz;
				
				alListenerfv(AL_ORIENTATION, orientation);
			}

			///ereditato da Manager
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

			ALuint sources[ MAX_SOURCES ];

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
