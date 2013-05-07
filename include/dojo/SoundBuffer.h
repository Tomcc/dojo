#ifndef SOUNDBUFFER_H
#define SOUNDBUFFER_H

#include "dojo_common_header.h"

#include "Resource.h"
#include "Stream.h"
#include "Array.h"
#include "Platform.h"

#include <atomic>

namespace Dojo 
{
	class SoundManager;

	///Internal class to contain a SoundBuffer loaded from file, or from memory
	/**
	Sounds are referenced using SoundSets, which contain one or more SoundBuffers, just like FrameSets and Textures.
	*/
	class SoundBuffer : public Resource
	{
	public:

		///A Chunk represents an actual OpenAL buffer. Chunks are used to preallocate streaming resources
		class Chunk : public Resource
		{
		public:

			///the max size in PCM of a chunk
			static const int MAX_DURATION = 3;
			static const int MAX_SIZE = 41000 * sizeof( short ) * MAX_DURATION;
			
			///Creates a new chunk that will use the given source span to load
			Chunk( SoundBuffer* parent, long streamStartPosition, long uncompressedSize ) :
				size( 0 ),
				alBuffer( AL_NONE ),
				references( 0 ),
				pParent( parent ),
				mStartPosition( streamStartPosition ),
				mUncompressedSize( uncompressedSize )
			{
				DEBUG_ASSERT( parent, "invalid parent" );
				DEBUG_ASSERT( streamStartPosition >= 0, "invalid starting position" );
				DEBUG_ASSERT( uncompressedSize > 0, "invalid PCM span size" );
			}

			~Chunk()
			{
				alDeleteBuffers( 1, &alBuffer );
			}

			///acquires one reference to this Chunk, and loads in a background thread
			void getAsync()
			{
				if( references++ == 0 && !isLoaded() ) //load it when referenced the first time
					loadAsync();
			}

			///acquires one reference to this Chunk, and loads it
			void get()
			{
				if( references++ == 0 && !isLoaded() ) //load it when referenced the first time
					onLoad();
			}

			///releases one reference to this Chunk, and unloads it if needed
			void release()
			{
				DEBUG_ASSERT( references >= 0, "References should never be less than 0" );

				if( --references == 0 ) //unload it when dereferenced last time
					onUnload();
			}

			///returns the underlying OpenAL buffer
			ALuint getOpenALBuffer()
			{
				DEBUG_ASSERT( isLoaded(), "This buffer is not loaded and has no AL buffer" );

				return alBuffer;
			}

			///returns the size of this chunk
			int getSize()
			{
				return size;
			}

			///loads in the default background queue
			void loadAsync();

			///loads the chunk asynchronously
			virtual bool onLoad();

			virtual void onUnload( bool soft = false );

		protected:

			SoundBuffer* pParent;
			long mStartPosition;
			long mUncompressedSize;

			ALuint size;
			ALuint alBuffer;
			int references;
		};

		typedef Array< Chunk* > ChunkList;
		
		///Creates a new file-loaded SoundBuffer in the given resourcegroup, for the given file path
		SoundBuffer( ResourceGroup* creator, const String& path );
		
		~SoundBuffer();

		virtual bool onLoad();
		virtual void onUnload( bool soft = false );

		///tells the memory size of this SoundBuffer
		inline int getSize()					{	return size;	}

		///returns the number of chunks that compose this sound
		inline int getChunkNumber()
		{
			return mChunks.size();
		}

		///tells the duration in seconds of this SoundBuffer
		inline float getDuration()				{	return mDuration;}

		inline bool isLoaded()					
		{
			return !mChunks.isEmpty();
		}

		virtual bool isReloadable()
		{
			return mSource != nullptr;
		}

		///tells if this buffer has been loaded as a streaming (multi-part, lazy-loaded) buffer.
		bool isStreaming()
		{
			return mChunks.size() > 1;
		}

		///get the nth part of this SoundBuffer. only streaming buffers have more than one part.
		/**
		if n is greater than the number of chunks, it will loop, just like animation frames
		*/
		Chunk* getChunk( int n, bool loadAsync = false )
		{
			DEBUG_ASSERT( n >= 0 && n < mChunks.size(), "The requested chunk is out of bounds" );

			if( loadAsync )
				mChunks[n]->getAsync();
			else
				mChunks[ n ]->get();

			return mChunks[ n ];
		}

	protected:

		static ov_callbacks VORBIS_CALLBACKS;

		static size_t _vorbisRead( void* out, size_t size, size_t count, void* source );
		static int _vorbisSeek( void *source, ogg_int64_t offset, int whence );			
		static int _vorbisClose( void *source );			
		static long _vorbisTell( void *source );

		ALuint size;
		float mDuration;

		ChunkList mChunks;
		Stream* mSource;

		bool _loadOgg( Stream* source );
		bool _loadOggFromFile();
	};
}

#endif
