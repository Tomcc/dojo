#ifndef SOUNDBUFFER_H
#define SOUNDBUFFER_H

#include "dojo_common_header.h"

#include "Buffer.h"

#include <ogg/ogg.h>

namespace Dojo 
{
	class SoundManager;

	///Classe che gestisce la memoria caricata da un wav.
	/**
	Non dovrebbe essere utilizzato all'esterno di SoundManager.
	*/
	class SoundBuffer : public Buffer
	{
	public:
		
		class VorbisSource
		{
		public:
			void* data;
			size_t size;
			
			long pointer;
			
			VorbisSource( void* d, size_t sz ) :
			data( d ),
			size( sz ),
			pointer( 0 )
			{
				DEBUG_ASSERT( data );
				DEBUG_ASSERT( size );
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

		bool load();
		void unload();

		void bind()
		{

		}

		///Ottieni la dimensione in memoria di questo buffer.
		inline int getSize()					{	return size;	}

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
		
		int _loadOggFromMemory( void* buf, int sz );
		int _loadOggFromFile();
						
		
	};
}

#endif
