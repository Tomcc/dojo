#ifndef SoundData_h__
#define SoundData_h__

#include "dojo_common_header.h"

#include "Array.h"
#include "SoundBuffer.h"

#include "dojomath.h"

namespace Dojo
{
	
	class SoundSet 
	{
	public:

		SoundSet( const String& setName ) :
		name( setName ),
		buffers( 1,1 )	//pagina minima, il vettore e' statico
		{
			
		}

		///restituisce un buffer casuale o quello numero "i"
		inline SoundBuffer* getBuffer( int i = -1 )
		{
			DEBUG_ASSERT( buffers.size() );
			DEBUG_ASSERT( (int)buffers.size() > i );

			if( i < 0 )
			{
				if( buffers.size() > 1 )
					i = (int)Math::rangeRandom( 0, (float)buffers.size() );
				else 
					i = 0;
			}

			return buffers.at(i);
		}

		inline uint getBufferNb()	{	return buffers.size();	 }

		inline const String& getName()	{	return name;	}

		inline void addBuffer( SoundBuffer* b )
		{
			DEBUG_ASSERT( b );
			
			buffers.add( b );
		}

	protected:

		String name;

		Array<SoundBuffer*> buffers;
	};
}

#endif
