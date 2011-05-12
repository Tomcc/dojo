#ifndef SoundData_h__
#define SoundData_h__

#include "dojo_common_header.h"

#include "Array.h"
#include "SoundBuffer.h"

#include "dojomath.h"
#include "BaseObject.h"

namespace Dojo
{
	
	class SoundSet : public BaseObject
	{
	public:

		SoundSet( const std::string& setName ) :
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

		inline const std::string& getName()	{	return name;	}

		inline void addBuffer( SoundBuffer* b )
		{
			DEBUG_ASSERT( b );
			
			buffers.add( b );
		}

	protected:

		std::string name;

		Array<SoundBuffer*> buffers;
	};
}

#endif
