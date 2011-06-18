#ifndef Serializable_h__
#define Serializable_h__

#include "dojo/dojo_common_header.h"

#include "dojo/Table.h"

namespace Dojo
{
		
	class Serializable
	{
	public:
		
		Serializable( uint uniqueID = 0 ) : 
		ID( uniqueID )
		{
			
		}

		inline void serialize( Table* t )
		{
			DEBUG_ASSERT( t );

			if( ID )
				t->setInteger( "ID", ID );

			onSerialize( t );
		}

		inline void deserialize( Table* t )
		{
			DEBUG_ASSERT( t );

			ID = t->getInt( "ID" );

			onDeserialize( t );
		}

		inline void setUniqueID( uint uniqueID )
		{
			ID = uniqueID;
		}
				
		inline uint getUniqueID()
		{
			return ID;
		}

	protected:

		virtual void onSerialize( Table* dest )=0;
		virtual void onDeserialize( Table* src )=0;

		uint ID;
	};
}

#endif