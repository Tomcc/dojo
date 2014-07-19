#ifndef SoundSource_h__
#define SoundSource_h__

#include "dojo_common_header.h"

#include "Vector.h"
#include "SoundBuffer.h"

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
				position = newPos;			
				positionChanged = true;
			}

			///sets sound volume, from 0 to 1
			/**
			with v > 1, hearing range still increases but the actual maximum volume does not
			*/
			void setVolume(float v);
			
			///sets the sound as looping. Looping sounds are never garbage collected.
			void setLooping(bool l);

			void setPitch( float p);
			
			///if autoremove is disabled, SoundManager won't garbage collect this Source
			void setAutoRemove(bool a)				{	autoRemove = a;		}	

			///Plays the sound with a given volume
			void play( float volume = 1.0f );
			
			void pause();

			///Stops the sound; it will be garbage collected from now on
			void stop()
			{
 				alSourceStop(source);
			}
			///Sets the playback to the beginning of the sound, and pauses it
			void rewind();

			///returns the Source's playing state
			SoundState getState()		{	return state;	}
			bool isPlaying()				{	return state == SS_PLAYING;	}

			///tells if this source is bound to a streaming SoundBuffer
			bool isStreaming()
			{
				return buffer && buffer->isStreaming();
			}

			ALuint getSource()			{	return source;	}

			float getVolume();	

			///returns the SoundBuffer that is currently being played
			SoundBuffer* getSoundBuffer()
			{
				return buffer;
			}

			///returns the elapsed time since source play 
			float getElapsedTime()
			{
				float elapsed = 0;
				alGetSourcef( source, AL_SEC_OFFSET, &elapsed );
				
				CHECK_AL_ERROR;

				return elapsed;
			}
			
			///is this a dummy sound?
			bool isValid()
			{
				return source != 0;
			}

			bool _isWaitingForDelete()
			{
				return (state == SS_FINISHED);
			}

			bool _isPaused()
			{
				return (state == SS_PAUSED);
			}

			void _update(float dt);

			void _setup( SoundBuffer* b )
			{
				DEBUG_ASSERT( b, "null SoundBuffer" );
				
				buffer = b;
			}

			bool isActive() const 
			{
				return state == SS_INITIALISING || state == SS_PAUSED || state == SS_PLAYING;
			}

			void _reset();
			
			void _loadResources();

		protected:

			typedef std::queue< SoundBuffer::Chunk* > ChunkQueue;
			
			Vector position, lastPosition;
			bool positionChanged;
			float timeSincePositionChange = 0;

			//members			
			SoundBuffer* buffer;
			ALuint source;
			ALint playState;

			int mCurrentChunkID, mQueuedChunks;
			SoundBuffer::Chunk* mFrontChunk, *mBackChunk;

			SoundState state;

			//params
			bool looping, autoRemove;	
			float baseVolume, pitch;
		};
}

#endif
