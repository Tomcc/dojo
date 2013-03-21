#ifndef SoundSource_h__
#define SoundSource_h__

#include "dojo_common_header.h"

#include "Vector.h"

namespace Dojo
{
		class SoundManager;
		class SoundBuffer;

		///SoundSource is an actual sound being played
		/**
		SoundSources are created (actually, they are drawn from a pool) using SoundManager::play(), and automatically
		collected when their playback ended - obviously except when the Source is a looping Source
		*/
		class SoundSource 
		{				
		public:

			enum SoundState 			
			{
				SS_INITIALISING, 
				SS_PLAYING,  
				SS_PAUSED,   
				SS_FINISHED  
			};
		
			///Internal constructor
			SoundSource( SoundManager* manager, ALuint source );

			virtual ~SoundSource();

			///sets the Source's position	
			void setPosition(const Vector& newPos)			
			{	
				pos = newPos;			
				positionChanged = true;
			}

			///sets sound volume, from 0 to 1
			/**
			with v > 1, hearing range still increases but the actual maximum volume does not
			*/
			void setVolume( float v );
			
			///sets the sound as looping. Looping sounds are never garbage collected.
			inline void setLooping(bool l)
			{
				looping = l;
				if( source ) alSourcei (source, AL_LOOPING, looping);
			}

			inline void setPitch( float p)			
			{		
				pitch = p;
				if( source ) alSourcef (source, AL_PITCH,  pitch);			
			}
			///if true, the buffer attached to this source will be destroyed when the playback ends!
			/** 
			useful for big one-shot sounds such as dialogue */
			inline void setFlushMemoryWhenRemoved(bool f)	{		flush = f;		}
			
			///if autoremove is disabled, SoundManager won't garbage collect this Source
			inline void setAutoRemove(bool a)				{	autoRemove = a;		}	

			///Plays the sound with a given volume
			void play( float volume = 1.0f );
			
			void pause();
			///Stops the sound; it will be garbage collected from now on
			inline void stop()
			{
				state = SS_FINISHED;
				alSourceStop(source);
			}
			///Sets the playback to the beginning of the sound, and pauses it
			void rewind();

			///returns the Source's playing state
			inline SoundState getState()		{	return state;	}
			inline bool isPlaying()				{	return state == SS_PLAYING;	}

			inline ALuint getSource()			{	return source;	}

			float getVolume();	

			///returns the elapsed time since source play 
			inline float getElapsedTime()
			{
				float elapsed = 0;
				alGetSourcef( source, AL_SEC_OFFSET, &elapsed );
				
				DEBUG_ASSERT( alGetError() == AL_NO_ERROR, "OpenAL error, cannot set source play offset" );

				return elapsed;
			}
			
			///is this a dummy sound?
			bool isValid()
			{
				return source != 0;
			}

			inline bool _isWaitingForDelete()
			{
				return (state == SS_FINISHED);
			}

			inline bool _isPaused()
			{
				return (state == SS_PAUSED);
			}

			void _update();

			void _setup( SoundBuffer* b )
			{
				DEBUG_ASSERT( b, "null SoundBuffer" );
				
				buffer = b;
			}

			void _reset();
			
			void _loadResources();

		protected:
			
			SoundManager* mgr;
			
			Vector pos;
			bool positionChanged;

			//members			
			SoundBuffer* buffer;
			ALuint source;
			ALfloat position[3];
			ALint playState;
			
			SoundState state;

			//params
			bool flush, looping, autoRemove;	
			float volume, pitch;
		};
}

#endif
