#ifndef Thread_h__
#define Thread_h__

#include "dojo_common_header.h"

#include "BaseObject.h"

#include "Runnable.h"

namespace Dojo 
{
	class Thread : public Dojo::BaseObject 
	{
	public:
		
		Thread( Runnable* r ) :
		runnable( r ),
		pthread( 0 )
		{
			DEBUG_ASSERT( runnable );
		}
		
		void start();
		
	protected:
		
		Runnable* runnable;
		
		pthread_t pthread;
		
		static void* threadStarter( void* main );
	};
}
#endif