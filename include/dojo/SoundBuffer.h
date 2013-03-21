#ifndef SOUNDBUFFER_H
#define SOUNDBUFFER_H

#include "dojo_common_header.h"

#include "Resource.h"

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
		
		class VorbisSource
		{
		public:
			void* data;
			long size;
			
			long pointer;
			
			VorbisSource( void* d, size_t sz ) :
			data( d ),
			size( sz ),
			pointer( 0 )
			{
				DEBUG_ASSERT( data, "null data passed" );
				DEBUG_ASSERT( size >= 0, "Invalid <= 0 size" );
			}
			
			static size_t read( void* out, size_t size, size_t count, void* source );
			static int seek( void *source, ogg_int64_t offset, int whence );			
			static int close( void *source );			
			static long tell( void *source );
		};

		///Costruttore
		SoundBuffer( ResourceGroup* creator, const String& path );
		///Distruttore
		~SoundBuffer();

		virtual bool onLoad();
		virtual void onUnload( bool soft = false );

		///Ottieni la dimensione in memoria di questo buffer.
		inline int getSize()					{	return size;	}
		inline float getDuration()				{	return mDuration;}

		inline bool isLoaded()					{	return buffer != AL_NONE;	}

		///-uso interno-
		inline ALuint _getOpenALBuffer()	
		{
			return buffer;
		}

	protected:
			
		SoundManager* mgr;
		
		ALsizei	size;

		ALuint buffer;

		float mDuration;
		
		int _loadOggFromMemory( void* buf, int sz );
		int _loadOggFromFile();
						
		
	};
}

#endif
