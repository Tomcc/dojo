#ifndef Table_h__
#define Table_h__

#include "dojo/dojo_common_header.h"

#include "dojo/BaseObject.h"
#include "dojo/Vector.h"
#include "dojo/Array.h"
#include "dojo/Utils.h"
#include "dojo/StringReader.h"
#include "dojo/String.h"

namespace Dojo
{
	class Table : public BaseObject 
	{
	public:

		enum FieldType 
		{
			FT_UNDEFINED,
			FT_NUMBER,
			FT_STRING,
			FT_DATA,
			FT_VECTOR,
			FT_TABLE
		};

		class Data
		{
		public:
			void* ptr;
			uint size;

			Data() :
			ptr( NULL ),
			size( 0 )
			{

			}

			Data( void* p, uint s ) :
			ptr( p ),
			size( s )
			{

			}
		};

		class Entry 
		{
		public:
			FieldType type;

			void* value;

			Entry() :
			type( FT_UNDEFINED ),
			value( NULL )
			{

			}

			Entry( FieldType fieldType, void* v ) :
			type( fieldType ),
			value( v )
			{
				
			}

			///need to call this before overwriting the entry
			void dispose()
			{
				delete value;
			}
		};

		typedef std::map< String, Entry > EntryMap;

		class Iterator : public EntryMap::iterator
		{
		public:

			Iterator( Table* t ) :
			iterator( t->map.begin() )
			{
				DEBUG_ASSERT( t );

				endItr = t->map.end();
			}

			inline boolean valid()
			{
				return (*this) != endItr;
			}

		protected:

			EntryMap::iterator endItr;
		};

		static Table EMPTY_TABLE;
		static const Data EMPTY_DATA;
		
		Table( const String& tablename = String::EMPTY ) :
		name( tablename ),
		unnamedMembers( 0 )
		{

		}
		
		~Table()
		{
			clear();
		}
		
		inline Table* createTable( const String& key )
		{	
			String name;

			if( key.size() == 0 )
				name = _getAutoName();
			else
				name = key;

			Table* t = new Table( name );			
			set( t );

			return t;
		}

		inline void setName( const String& newName )
		{
			DEBUG_ASSERT( newName.size() > 0 );

			name = newName;
		}

		inline void set( const String& key, FieldType type, void* value )
		{
			DEBUG_ASSERT( type == FT_NUMBER || (value != NULL) );

			//generate name
			if( key.size() == 0 )
				map[ _getAutoName() ] = Entry( type, value );
			else
			{
				if( exists( key ) )
					map[ key ].dispose();

				map[ key ] = Entry( type, value );
			}
		}
		
		inline void set( const String& key, float value )
		{			
			set(key, FT_NUMBER, new float( value ) );
		}

		inline void set( const String& key, int value )
		{
			set( key, (float)value );
		}

		inline void set( const String& key, uint value )
		{
			set( key, (float)value );
		}

		///boolean has to be specified as C has the ugly habit of casting everything to it without complaining
		inline void setBoolean( const String& key, bool value )
		{
			set( key, (float)value );
		}
		
		inline void set( const String& key, const String& value )
		{			
			set(key, FT_STRING, new String( value ) );
		}

		inline void set( const String& key, const Vector& value )
		{
			set( key, FT_VECTOR, new Vector( value ) );
		}

		///WARNING - Data DOES NOT ACQUIRE OWNERSHIP OF THE DATA!!!
		inline void set( const String& key, void* value, uint size )
		{
			DEBUG_ASSERT( value );
			DEBUG_ASSERT( size );

			set(key, FT_DATA, new Data( value, size ) );
		}
		
		inline void set( Table* value )
		{
			DEBUG_ASSERT( value );
						
			set( value->getName(), FT_TABLE, value);
		}		
		
		void clear()
		{					
			//clean up every entry
			
			for( Iterator itr = getIterator(); itr.valid(); ++itr )
				itr->second.dispose();
			
			map.clear();
		}		
		
		inline const String& getName()
		{
			return name;
		}

		inline uint getAutoMembers()
		{
			return unnamedMembers;
		}

		inline boolean hasName() 
		{
			return name.size() > 0;
		}

		inline bool exists( const String& key )
		{
			DEBUG_ASSERT( key.size() );

			return map.find( key ) == map.end();
		}

		inline bool existsAs( const String& key, FieldType t )
		{
			EntryMap::iterator itr = map.find( key );

			return itr != map.end() && itr->second.type == t;
		}
		
		inline float getNumber( const String& key )
		{			
			if( existsAs( key, FT_NUMBER ) )
				return *( (float*)map[key].value );
			else
				return 0;
		}

		inline int getInt( const String& key )
		{
			return (int)getNumber(key);
		}

		inline bool getBool( const String& key )
		{
			return getNumber(key) > 0.f;
		}
		
		inline const String& getString( const String& key )
		{
			if( existsAs(key, FT_STRING ) )
				return *( (String*)map[key].value );
			else
			   return String::EMPTY;
		}

		inline const Dojo::Vector& getVector( const String& key )
		{
			if( existsAs( key, FT_VECTOR ) ) 
				return *( (Vector*)map[key].value );
			else
				return Vector::ZERO;
		}

		inline const Dojo::Color getColor( const String& key, float alpha = 1.f )
		{
			return Color( getVector( key ), alpha );
		}
		
		inline Table* getTable( const String& key )
		{			
			if( existsAs(key, FT_TABLE ) )
			   return (Table*)map[key].value;
		   else
			   return &EMPTY_TABLE;
		}

		inline const Data& getData( const String& key )
		{
			if( existsAs( key, FT_DATA ) )
				return *( (Data*)map[ key ].value );
			else
				return EMPTY_DATA;
		}	

		inline String autoMember( uint i )
		{
			DEBUG_ASSERT( i < getAutoMembers() );

			return '_' + String( (int)i );
		}

		inline const Iterator getIterator()
		{
			return Iterator( this );
		}

		inline bool isEmpty()
		{
			return map.size() == 0;
		}
		
		///scrive la tabella in un formato standard su stringa che inizia a pos
		void serialize( String& buf, String indent = String::EMPTY );

		void deserialize( StringReader& buf );
				
	protected:
		
		String name;
		
		EntryMap map;

		uint unnamedMembers;

		inline String _getAutoName()
		{
			return '_' + String( unnamedMembers++ );
		}
	};
}


#endif
