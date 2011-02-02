#ifndef Serializable_h__
#define Serializable_h__

#include "dojo_common_header.h"

namespace Dojo
{
	class Table;
	
	class Serializable
	{
	public:
		
		Serializable()
		{
			
		}
		
		virtual void onSerialize( Table* dest )=0;
		
		virtual void onDeserialize( Table* src )=0;
		
	protected:
	};
}

#endif