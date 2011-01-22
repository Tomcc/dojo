#include "Thread.h"

using namespace Dojo;

void* Thread::threadStarter( void* runnable )
{
	Runnable* r = (Runnable*)runnable;
	
	r->run();
}

void Thread::start()
{
	DEBUG_ASSERT( !pthread );
	
	//call thread starter
	
	int error = pthread_create( 
				   &pthread,
				   NULL, 
				   Thread::threadStarter, 
				   (void*)runnable );
	
	//crasha sonoramente se non riesce
	DEBUG_ASSERT( error == 0 ); 
}