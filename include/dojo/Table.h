#ifndef Table_h__
#define Table_h__

#include "dojo_common_header.h"

#include "BaseObject.h"
#include "Vector.h"
#include "Array.h"
#include "Utils.h"

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

		typedef std::map< std::string, Entry > EntryMap;

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

		static const std::string UNDEFINED_STRING;
		static Table EMPTY_TABLE;
		static const Data EMPTY_DATA;
		
		Table( const std::string& tablename = "" ) :
		name( tablename ),
		unnamedMembers( 0 )
		{

		}
		
		///legge la tabella dal formato standard su stringa
		Table( std::istream& buf )
		{	
			deserialize( buf );
		}		
		
		~Table()
		{
			clear();
		}
		
		inline Table* createTable( const std::string& key )
		{			
			Table* t = new Table( key );
			
			setTable( t );
			
			return t;
		}

		inline void setName( const std::string& newName )
		{
			DEBUG_ASSERT( newName.size() > 0 );

			name = newName;
		}

		inline void setRaw( const std::string& key, FieldType type, void* value )
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
		
		inline void setNumber( const std::string& key, float value )
		{			
			setRaw(key, FT_NUMBER, new float( value ) );
		}

		inline void setInteger( const std::string& key, int value )
		{
			setNumber( key, (int)value );
		}

		inline void setBoolean( const std::string& key, bool value )
		{
			setNumber( key, (float)value );
		}
		
		inline void setString( const std::string& key, const std::string& value )
		{			
			setRaw(key, FT_STRING, new std::string( value ) );
		}

		inline void setVector( const std::string& key, const Vector& value )
		{
			setRaw( key, FT_VECTOR, new Vector( value ) );
		}

		///WARNING - Data DOES NOT ACQUIRE OWNERSHIP OF THE DATA and requires it to be preserved!
		inline void setData( const std::string& key, void* value, uint size )
		{
			DEBUG_ASSERT( value );
			DEBUG_ASSERT( size );

			setRaw(key, FT_DATA, new Data( value, size ) );
		}
		
		inline void setTable( Table* value )
		{
			DEBUG_ASSERT( value );
						
			setRaw( value->getName(), FT_TABLE, value);
		}		
		
		void clear()
		{					
			//clean up every entry
			
			for( Iterator itr = getIterator(); itr.valid(); ++itr )
				itr->second.dispose();
			
			map.clear();
		}		
		
		inline const std::string& getName()
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

		inline bool exists( const std::string& key )
		{
			DEBUG_ASSERT( key.size() );

			return map.find( key ) == map.end();
		}

		inline bool existsAs( const std::string& key, FieldType t )
		{
			EntryMap::iterator itr = map.find( key );

			return itr != map.end() && itr->second.type == t;
		}
		
		inline float getNumber( const std::string& key )
		{			
			if( existsAs( key, FT_NUMBER ) )
				return *( (float*)map[key].value );
			else
				return 0;
		}

		inline int getInt( const std::string& key )
		{
			return (int)getNumber(key);
		}

		inline bool getBool( const std::string& key )
		{
			return getNumber(key) > 0.f;
		}
		
		inline const std::string& getString( const std::string& key )
		{
			if( existsAs(key, FT_STRING ) )
				return *( (std::string*)map[key].value );
			else
			   return UNDEFINED_STRING;
		}

		inline const Dojo::Vector& getVector( const std::string& key )
		{
			if( existsAs( key, FT_VECTOR ) ) 
				return *( (Vector*)map[key].value );
			else
				return Vector::ZERO;
		}

		inline const Dojo::Color getColor( const std::string& key, float alpha = 1.f )
		{
			return Color( getVector( key ), alpha );
		}
		
		inline Table* getTable( const std::string& key )
		{			
			if( existsAs(key, FT_TABLE ) )
			   return (Table*)map[key].value;
		   else
			   return &EMPTY_TABLE;
		}

		inline const Data& getData( const std::string& key )
		{
			if( existsAs( key, FT_DATA ) )
				return *( (Data*)map[ key ].value );
			else
				return EMPTY_DATA;
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
		void serialize( std::ostream& buf, std::string indent );

		void deserialize( std::istream& buf );
				
	protected:
		
		std::string name;
		
		EntryMap map;

		uint unnamedMembers;

		inline std::string _getAutoName()
		{
			return "_" + Utils::toString( unnamedMembers++ );
		}
	};
}


#endif
