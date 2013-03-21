#ifndef SoundData_h__
#define SoundData_h__

#include "dojo_common_header.h"

#include "Array.h"
#include "Resource.h"
#include "dojomath.h"

namespace Dojo
{
	class SoundBuffer;
	
	///A SoundSet is a collection that contains and abstracts one or more SoundBuffers
	/**the abstraction is useful when it's needed to play a set of sounds randomly, eg, collision sounds.

	To define a SoundSet with more than 1 Buffer in it, it's just needed to name sounds with a numeric tag, ie:
	step_1.ogg
	step_2.ogg
	step_3.ogg */
	class SoundSet : public Resource
	{
	public:

		///Creates a new set named setName
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
			DEBUG_ASSERT( buffers.size(), "This SoundSet is empty" );
			DEBUG_ASSERT_INFO( (int)buffers.size() > i, "Trying to get an OOB sound index", "index = " + String( i ) );

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
			DEBUG_ASSERT( b, "Adding a NULL SoundBuffer" );
			
			buffers.add( b );
		}

	protected:

		String name;

		Array<SoundBuffer*> buffers;
	};
}

#endif
