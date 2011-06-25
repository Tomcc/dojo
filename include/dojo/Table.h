#ifndef Table_h__
#define Table_h__

#include "dojo_common_header.h"

#include "BaseObject.h"
#include "Vector.h"
#include "Array.h"
#include "Utils.h"
#include "StringReader.h"
#include "dojostring.h"

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
			
			Entry( FieldType fieldType ) :
			type( fieldType )
			{
				DEBUG_ASSERT( type <= FT_TABLE );
			}
					
			virtual ~Entry()
			{
				
			}
			
			virtual void* getValue()=0;
		};
		
		template <class T>
		class TypedEntry : public Entry
		{
		public:

			T value;

			TypedEntry( FieldType fieldType, const T& v ) :
			Entry( fieldType ), 
			value( v )
			{
				
			}
					
			virtual ~TypedEntry()
			{
				
			}
			
			virtual void* getValue()
			{
				return &value;
			}
		};

		typedef std::map< String, Entry* > EntryMap;

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
		
		inline Table* createTable( const String& key = String::EMPTY )
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

		template< class T >
		inline void set( const String& key, FieldType type, const T& value )
		{
			DEBUG_ASSERT( key.size() );
			
			//generate name
			if( key.size() == 0 )
				map[ _getAutoName() ] = new TypedEntry< T >( type, value );
			else
			{
				if( exists( key ) )
					delete map[key];

				map[ key ] = new TypedEntry< T >( type, value );
			}
		}
		
		inline void set( const String& key, float value )
		{			
			set(key, FT_NUMBER, value );
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
			set(key, FT_STRING, value );
		}

		inline void set( const String& key, const Vector& value )
		{
			set( key, FT_VECTOR, value );
		}

		///WARNING - Data DOES NOT ACQUIRE OWNERSHIP OF THE DATA!!!
		inline void set( const String& key, void* value, uint size )
		{
			DEBUG_ASSERT( value );
			DEBUG_ASSERT( size );

			set(key, FT_DATA, Data( value, size ) );
		}
		
		inline void set( Table* value )
		{
			DEBUG_ASSERT( value );
						
			set( value->getName(), FT_TABLE, value );
		}		
		
		void clear()
		{					
			//clean up every entry
			EntryMap::iterator itr = map.begin();
			
			for( ; itr != map.end(); ++itr )
				delete itr->second;
			
			map.clear();
		}		
		
		inline const String& getName() const
		{
			return name;
		}

		inline uint getAutoMembers() const
		{
			return unnamedMembers;
		}

		inline bool hasName() const
		{
			return name.size() > 0;
		}

		inline bool exists( const String& key ) const
		{
			DEBUG_ASSERT( key.size() );

			return map.find( key ) != map.end();
		}

		inline bool existsAs( const String& key, FieldType t ) const
		{
			EntryMap::const_iterator itr = map.find( key );
						
			if( itr != map.end() )
			{
				Entry* e = itr->second;
				return e->type == t;
			}
			return false;
		}
		
		inline Entry* get( const String& key ) const
		{
			return map.find( key )->second;
		}
		
		inline float getNumber( const String& key ) const
		{			
			if( existsAs( key, FT_NUMBER ) )
				return *( (float*)get(key)->getValue() );
			else
				return 0;
		}

		inline int getInt( const String& key ) const
		{
			return (int)getNumber(key);
		}

		inline bool getBool( const String& key ) const
		{
			return getNumber(key) > 0.f;
		}
		
		inline const String& getString( const String& key ) const
		{
			if( existsAs(key, FT_STRING ) )
				return *( (String*)get(key)->getValue() );
			else
			   return String::EMPTY;
		}

		inline const Dojo::Vector& getVector( const String& key ) const
		{
			if( existsAs( key, FT_VECTOR ) ) 
				return *( (Vector*)get(key)->getValue() );
			else
				return Vector::ZERO;
		}

		inline const Dojo::Color getColor( const String& key, float alpha = 1.f ) const
		{
			return Color( getVector( key ), alpha );
		}
		
		inline Table* getTable( const String& key ) const
		{			
			if( existsAs(key, FT_TABLE ) )
			   return *( (Table**)get(key)->getValue());
		   else
			   return &EMPTY_TABLE;
		}

		inline const Data& getData( const String& key ) const
		{
			if( existsAs( key, FT_DATA ) )
				return *( (Data*)get(key)->getValue() );
			else
				return EMPTY_DATA;
		}	

		inline String autoMember( uint i ) const
		{
			DEBUG_ASSERT( i < getAutoMembers() );

			return '_' + String( (int)i );
		}

		inline bool isEmpty() const
		{
			return map.size() == 0;
		}
		
		///scrive la tabella in un formato standard su stringa che inizia a pos
		void serialize( String& buf, String indent = String::EMPTY ) const;

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
