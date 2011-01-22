#ifndef SOUNDMANAGER_H_
#define SOUNDMANAGER_H_

#include "dojo_common_header.h"

#include "Array.h"
#include "Vector.h"

#include "SoundBuffer.h"
#include "SoundSet.h"
#include "SoundSource.h"
#include "BaseObject.h"

namespace Dojo {

		class SoundListener;
		class SoundSource;

		///Classe che gestisce il sistema Audio di eVolve.
		class SoundManager : public BaseObject
		{
		protected:

			class SoundDummy : public SoundSource
			{

			public:
				
				SoundDummy( SoundManager* mgr ) :
				SoundSource( mgr )
				{
					
				}

				bool isValid()
				{
					return false;
				}
			};


		public:

			static const float m = 100;
						
			static const uint maxBuffers = 32;
			static const uint maxSources = 64;

			typedef Array<SoundSource*> SoundList;
			typedef std::map<std::string, SoundSet*> SoundDataMap;

			///metodo statico per convertire la classe vector in vettore C-style.
			inline static void vectorToALfloat(const Vector& vector, ALfloat* ALpos )
			{
				DOJO_ASSERT( ALpos );
				
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
					SoundSource* s = busySoundPool.popElement();
					s->stop();
					delete s;
				}
				
				musicTrack = false;
				fadeState = FS_NONE;
			}

			//restituisce un suono che non fa nulla
			inline SoundSource* getDummySound()
			{
				return dummySound;
			}

			///restituisce una fonte sonora con il suono dato
			inline SoundSource* getSoundSource( SoundSet* set )
			{
				if( idleSoundPool.size() > 0 && set )
				{
					SoundSource* s = idleSoundPool.popElement();
					busySoundPool.addElement(s);

					s->_setup( set->getBuffer() );

					return s;
				}

				return dummySound;
			}

			///restituisce una fonte sonora con il suono dato
			inline SoundSource* getSoundSource(const std::string& bufferName)
			{
				SoundSet* s = getSoundSet( bufferName );
				if( s )
					return getSoundSource( s );

				//suono non trovato!
				return dummySound;
			}

			inline SoundSource* getSoundSource( const Vector& pos, SoundSet* set )
			{
				DOJO_ASSERT( set );
				
				SoundSource* s = getSoundSource( set );
				s->setPosition( pos );
				return s;
			}

			inline SoundSource* getSoundSource( const Vector& pos, const std::string& bufferName)
			{
				SoundSource* s = getSoundSource( bufferName );
				s->setPosition( pos );
				return s;
			}

			inline SoundSource* playSound( SoundSet* set )
			{
				DOJO_ASSERT( set );
				
				SoundSource* s = getSoundSource( set );
				s->play();
				return s;
			}

			inline SoundSource* playSound(const std::string& bufferName)
			{
				SoundSource* s = getSoundSource( bufferName );
				s->play();
				return s;
			}

			inline SoundSource* playSound( const Vector& pos, SoundSet* set )
			{
				DOJO_ASSERT( set );
				
				SoundSource* s = getSoundSource( pos, set );
				s->play();
				return s;
			}

			inline SoundSource* playSound( const Vector& pos, const std::string& bufferName )
			{
				SoundSource* s = getSoundSource( pos, bufferName );
				s->play();
				return s;
			}
			
			///tells whether the system is already playing its sound (iTunes, calls, VOIP, etc)
			bool isSystemSoundInUse();
			
			///setta la musica facendo un fade lineare in fadeTime rispetto alla track precedente
			void playMusic( const std::string& name, float trackFadeTime = 0 );
			
			inline void pauseMusic()
			{
				if( musicTrack )
					musicTrack->pause();
			}
			
			inline void resumeMusic()
			{
				//resume music, but only if the user didn't enable itunes meanwhile!
				if( musicTrack && !isSystemSoundInUse() )
					musicTrack->play();
			}

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
				DOJO_ASSERT( volume >= 0 );
				
				musicVolume = volume;	

				if( !nextMusicTrack && musicTrack )
					musicTrack->setVolume( musicVolume );
			}
			
			inline void setMasterVolume( float volume )	
			{	
				DOJO_ASSERT( volume >= 0 );
				
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

			///Dice se esiste un buffer creato dal file specificato.
			inline bool isSoundLoaded(const std::string& name)
			{
				return soundDataMap.find( name ) != soundDataMap.end();
			}

			///Aggiunge manualmente un set di suoni, utile per gruppi che non sono nel filesystem
			inline void addSoundSet( SoundSet* s )
			{
				DOJO_ASSERT( s );
				
				if( !isSoundLoaded( s->getName()) )
					soundDataMap[ s->getName() ] = s;
			}

			inline SoundSet* getSoundSet( const std::string& name )
			{
				//esiste un set col nome dato?
				SoundDataMap::iterator itr = soundDataMap.find( name );

				if( itr != soundDataMap.end() )
					return itr->second;

				return NULL;
			}
			
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

			///-metodo interno- mantiene il numero di buffers caricati minore del massimo
			bool _reserveBufferSlot( SoundBuffer* newbuf )
			{
				DOJO_ASSERT( newbuf );
				
				if( loadedBufferList.size() >= maxBuffers )
				{
					bool removed = false;
					//rimuovi un buffer senza referenze
					for( unsigned int i = 0; i < loadedBufferList.size(); ++i )
					{
						if( loadedBufferList.at(i)->getUses() == 0 )
						{
							loadedBufferList.removeElement( i );

							removed = true;
							break;
						}
					}

					//non ce ne sono...
					if( !removed )
						return false;
				}

				loadedBufferList.addElement( newbuf );			
				return true;
			}

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
			SoundSource* dummySound;
			SoundList idleSoundPool;
			SoundList busySoundPool;

			SoundSource *musicTrack, *nextMusicTrack;
			float halfFadeTime;
			float currentFadeTime;
			FadeState fadeState;

			float musicVolume;
			float masterVolume;

			//mappa di liste di buffers per i set
			SoundDataMap soundDataMap;

			Array<SoundBuffer*> loadedBufferList; 

			void _createMainBundleBuffers();		
			
		};
}

#endif
