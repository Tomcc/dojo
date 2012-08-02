#ifndef SoundData_h__
#define SoundData_h__

#include "dojo_common_header.h"

#include "Array.h"
#include "Resource.h"
#include "dojomath.h"

namespace Dojo
{
	class SoundBuffer;
	
	class SoundSet : public Resource
	{
	public:

		SoundSet( ResourceGroup* creator, const String& setName ) :
		Resource( creator ),
		name( setName ),
		buffers( 1,1 )	//pagina minima, il vettore e' statico
		{
			
		}

		virtual bool onLoad();
		virtual void onUnload( bool soft = true );

		///returns a random buffer (-1) or the one at index i
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

		inline uint getResourceNb()	{	return buffers.size();	 }

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
