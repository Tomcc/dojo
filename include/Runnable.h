#ifndef Runnable_h__
#define Runnable_h__

#include "dojo_common_header.h"

namespace Dojo 
{
	class Runnable
	{
	public:
		
		Runnable()
		{
			
		}
		
		virtual void run()=0;
		
	protected:
	};
}

#endif