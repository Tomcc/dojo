#ifndef Table_h__
#define Table_h__

#include "dojo_common_header.h"

#include "Vector.h"
#include "Array.h"
#include "Utils.h"
#include "StringReader.h"
#include "dojostring.h"

namespace Dojo
{
	class Table  
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

			Data( void* p, uint s) :
			ptr( p ),
			size( s )
			{

			}
			
			~Data()
			{
				
			}			
		};
		
		class Entry
		{
		public:
			FieldType type;
			bool autoRelease;
			
			Entry( FieldType fieldType ) :
			type( fieldType ),
			autoRelease( false )
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
				if( autoRelease )
				{
					if( type == FT_TABLE ) //delete the table
						delete *((Table**)(&value));
					
					else if( type == FT_DATA ) //delete data's payload
						free( ((Data*)(&value))->ptr );
				}
			}
			
			virtual void* getValue()
			{
				return &value;
			}
		};

		typedef std::map< String, Entry* > EntryMap;

		static Table EMPTY_TABLE;
		static const Data EMPTY_DATA;
		
		inline static String index( uint i )
		{
			return '_' + String(i);
		}
		
		Table( const String& tablename = String::EMPTY ) :
		name( tablename ),
		unnamedMembers( 0 )
		{

		}
		
		~Table()
		{
			clear();
		}
		
		inline void setName( const String& newName )
		{
			DEBUG_ASSERT( newName.size() > 0 );

			name = newName;
		}

		template< class T >
		inline void set( const String& key, FieldType type, const T& value )
		{			
			//generate name
			if( key.size() == 0 )
				map[ _getAutoName() ] = new TypedEntry< T >( type, value );
			else
			{
				if( exists( key ) )
					SAFE_DELETE( map[key] );

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
		
		inline void set( const String& key, const Color& value )
		{
			set( key, FT_VECTOR, Vector( value.r, value.g, value.b ) );
		}

		///WARNING - Data DOES NOT ACQUIRE OWNERSHIP OF THE DATA!!!
		inline void set( const String& key, void* value, uint size, bool managed = false )
		{
			DEBUG_ASSERT( value );
			DEBUG_ASSERT( size );

			set(key, FT_DATA, Data( value, size ) );
			
			//set it managed
			if( managed )
				map[ key ]->autoRelease = true;
		}
		
		inline void set( Table* value, bool managed = false )
		{
			DEBUG_ASSERT( value );
						
			set( value->getName(), FT_TABLE, value );
						
			//set it managed
			if( managed )
				map[ value->getName() ]->autoRelease = true;
		}		
		
		inline Table* createTable( const String& key = String::EMPTY )
		{	
			String name;
			
			if( key.size() == 0 )
				name = _getAutoName();
			else
				name = key;
			
			Table* t = new Table( name );			
			set( t, true ); //always retain created tables
			
			return t;
		}
		
		void clear()
		{					
			unnamedMembers = 0;
			
			//clean up every entry
			EntryMap::iterator itr = map.begin();
			
			for( ; itr != map.end(); ++itr )
				SAFE_DELETE( itr->second );
			
			map.clear();
		}		
		
		inline int size()
		{
			return (int)map.size();
		}
		
		inline const String& getName() const
		{
			return name;
		}

		inline int getAutoMembers() const
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
		
		inline String autoMemberName( int idx ) const 
		{
			DEBUG_ASSERT( idx < getAutoMembers() );
			
			return '_' + String( idx );
		}
		
		inline float getNumber( uint idx ) const
		{			
			return getNumber( autoMemberName( idx ) );
		}
		
		inline int getInt( uint idx ) const
		{
			return (int)getNumber( idx );
		}
		
		inline bool getBool( uint idx ) const
		{
			return getNumber(idx) > 0.f;
		}
		
		inline const String& getString( uint idx ) const
		{
			return getString( autoMemberName(idx) );
		}
		
		inline const Dojo::Vector& getVector( uint idx ) const
		{
			return  getVector( autoMemberName(idx ) );
		}
		
		inline const Dojo::Color getColor( uint idx, float alpha = 1.f ) const
		{
			return Color( getVector( idx ), alpha );
		}
		
		inline Table* getTable( uint idx ) const
		{			
			return getTable( autoMemberName(idx) );
		}
		
		inline const Data& getData( uint idx ) const
		{
			return getData( autoMemberName( idx ) );
		}	
		
		inline const EntryMap::const_iterator begin() const
		{
			return map.begin();
		}
		
		inline const EntryMap::const_iterator end() const
		{
			return map.end();
		}
		
		///removes a member named key
		inline void remove( const Dojo::String& key )
		{
			map.erase( key );
		}
		
		///removes the unnamed member index idx
		inline void remove( int idx )
		{
			map.erase( autoMemberName( idx ) );
		}

		inline bool isEmpty() const
		{
			return map.size() == 0;
		}
		
		///scrive la tabella in un formato standard su stringa che inizia a pos
		void serialize( String& buf, String indent = String::EMPTY ) const;

		void deserialize( StringReader& buf );
		
		///diagnostic method that serializes the table in a string
		inline String toString()
		{
			String str = getName() + '\n';
			serialize( str );
			
			return str;
		}
		
		inline void debugPrint()
		{
#ifdef _DEBUG			
			DEBUG_MESSAGE( toString().ASCII() );
#endif
		}
				
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
