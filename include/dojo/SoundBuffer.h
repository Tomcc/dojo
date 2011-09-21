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
			
			static size_t read( void* out, size_t size, size_t count, void* source )
			{
				VorbisSource* src = (VorbisSource*)source;
				
				int bytes = size * count;
				int max = src->size - src->pointer;

				if( bytes > max ) bytes = max;
				
				if( bytes > 0 )
				{
					memcpy( out, (char*)src->data + src->pointer, bytes );
					
					src->pointer += bytes;
				}
				
				return bytes / size;
			}
			
			static int seek( void *source, ogg_int64_t offset, int whence )
			{
				VorbisSource* src = (VorbisSource*)source;
				
				if( whence == SEEK_SET )
					src->pointer = offset;
				else if( whence == SEEK_END )
					src->pointer = src->size - offset;
				else if( whence == SEEK_CUR )
					src->pointer += offset;
				else
				{
					DEBUG_TODO;
				}
				
				return 0;
			}
			
			static int close( void *source )
			{
				//do nothing
				
				return 0;
			}
			
			static long tell( void *source )
			{
				return ((VorbisSource*)source)->pointer;
			}
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
