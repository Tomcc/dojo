#ifndef SoundSource_h__
#define SoundSource_h__

#include "dojo_common_header.h"

#include "Vector.h"
#include "SoundBuffer.h"

#include <queue>

namespace Dojo
{
		class SoundManager;
		
		///SoundSource is an actual sound being played
		/**
		SoundSources are created (actually, they are drawn from a pool) using SoundManager::play(), and automatically
		collected when their playback ended - obviously except when the Source is a looping Source
		*/
		class SoundSource 
		{				
		public:

			static const int QUEUE_SIZE = 3;

			enum SoundState 			
			{
				SS_INITIALISING, 
				SS_PLAYING,  
				SS_PAUSED,   
				SS_FINISHED  
			};
		
			///Internal constructor
			SoundSource( ALuint source );

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
				if( source ) //do not use this looping flag on streaming sounds, we handle it in the update 
					alSourcei (source, AL_LOOPING, isStreaming() ? false : looping );
			}

			inline void setPitch( float p)			
			{		
				pitch = p;
				if( source ) alSourcef (source, AL_PITCH,  pitch);			
			}
			
			///if autoremove is disabled, SoundManager won't garbage collect this Source
			inline void setAutoRemove(bool a)				{	autoRemove = a;		}	

			///Plays the sound with a given volume
			void play( float volume = 1.0f );
			
			void pause();

			///Stops the sound; it will be garbage collected from now on
			inline void stop()
			{
 				alSourceStop(source);
			}
			///Sets the playback to the beginning of the sound, and pauses it
			void rewind();

			///returns the Source's playing state
			inline SoundState getState()		{	return state;	}
			inline bool isPlaying()				{	return state == SS_PLAYING;	}

			///tells if this source is bound to a streaming SoundBuffer
			bool isStreaming()
			{
				return buffer && buffer->isStreaming();
			}

			inline ALuint getSource()			{	return source;	}

			float getVolume();	

			///returns the SoundBuffer that is currently being played
			SoundBuffer* getSoundBuffer()
			{
				return buffer;
			}

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

			typedef std::queue< SoundBuffer::Chunk* > ChunkQueue;
			
			Vector pos;
			bool positionChanged;

			//members			
			SoundBuffer* buffer;
			ALuint source;
			ALfloat position[3];
			ALint playState;

			int mCurrentChunkID;
			SoundBuffer::Chunk* mCurrentChunk;
			ChunkQueue mChunkQueue;

			SoundState state;

			//params
			bool looping, autoRemove;	
			float volume, pitch;
		};
}

#endif
