#include "stdafx.h"

#include "Thread.h"

using namespace Dojo;

void* Thread::threadStarter( void* runnable )
{
	Runnable* r = (Runnable*)runnable;
	
	r->run();
}

void Thread::start()
{
	DEBUG_ASSERT( !nativeThread );
	
	//call thread starter

#ifdef WIN32

	nativeThread = CreateThread(
		NULL,
		NULL,
		Thread::threadStarter,
		(void*)runnable,
		NULL,
		NULL );

	DEBUG_ASSERT( nativeThread );

#else
	int error = pthread_create( 
				   &nativeThread,
				   NULL, 
				   Thread::threadStarter, 
				   (void*)runnable );
	
	//crasha sonoramente se non riesce
	DEBUG_ASSERT( error == 0 ); 

#endif
}