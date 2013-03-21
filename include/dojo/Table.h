#ifndef Table_h__
#define Table_h__

#include "dojo_common_header.h"

#include "Vector.h"
#include "Array.h"
#include "Utils.h"
#include "StringReader.h"
#include "dojostring.h"
#include "Resource.h"

namespace Dojo
{
	///Table is the internal representation of the Dojo Script data definition format
	/** 
	a Table is a multi-typed Dictionary of Strings and Values, where a value can be one of float, Vector, String, Color, Raw Data and Table itself.
	
	set( key, value ) sets a value to the given key;
	get*( key, defaultValue ) gets the value of the given key, or returns defaultValue if the key was not found 
	
	Table does support numeric indexing via "auto values", or values which are not bound to a (explicit) name.
	auto values can be queried using
	get*( index, defaultValue )
	*/
	class Table : public Resource
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
			
			Entry( FieldType fieldType ) :
			type( fieldType )
			{

			}
					
			virtual ~Entry()
			{
				
			}
								
			virtual void* getValue()=0;

			virtual Entry* clone()=0;
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
			
			virtual Entry* clone()
			{
				return new TypedEntry<T>(type, value );
			}
		};

		typedef std::unordered_map< String, Entry* > EntryMap;

		static Table EMPTY_TABLE;
		static const Data EMPTY_DATA;
		
		inline static String index( uint i )
		{
			return '_' + String(i);
		}

		///loads the file at path into dest
		static void loadFromFile( Table* dest, const String& path );
		
		///Creates a new table optionally named tablename
		Table( const String& tablename = String::EMPTY ) :
		Resource(),
		name( tablename ),
		unnamedMembers( 0 )
		{

		}

		///copy constructor
		Table( const Table& t ) :
		Resource(),
		name( t.name ),
		unnamedMembers( t.unnamedMembers )
		{
			EntryMap::const_iterator itr = t.map.begin(),
								end = t.map.end();

			//do a deep copy
			for( ; itr != end; ++itr )
				map[ itr->first ] = itr->second->clone();
		}

		///Constructs a new "Table Resource", or a table bound to a file path in a ResourceGroup
		Table( ResourceGroup* creator, const String& path ) :
		Resource( creator, path ),
		name( Utils::getFileName( path ) ),
		unnamedMembers( 0 )
		{

		}

		~Table()
		{
			clear();
		}

		virtual bool onLoad();

		virtual void onUnload( bool soft = false )
		{
			if( !soft || isReloadable() )
			{
				clear();

				loaded = false;
			}
		}
		
		///sets the Table name
		inline void setName( const String& newName )
		{
			DEBUG_ASSERT( newName.size() > 0, "Setting an empty Table name" );

			name = newName;
		}

		///returns the table which contains the given "dot formatted" key
		/** it returns "this" for a normal non-hierarchical key
		returns "A" for a key such as "A.key"
		returns "B" for a key such as "A.B.key" */
		Table* getParentTable( const String& key, String& realKey ) const
		{
			int dotIdx = 0;
			for(; dotIdx< key.size() && key[dotIdx] != '.'; ++dotIdx ); 

			if( dotIdx == key.size() )
			{
				realKey = key;
				return (Table*)this;
			}

			String partialKey = key.substr( dotIdx+1 );
			String childName = key.substr( 0, dotIdx );
			Table* child = getTable( childName );

			DEBUG_ASSERT_INFO( child->size() > 0, "A part of a dot-formatted key referred to a non-existing table", "childName = " + childName );

			return child->getParentTable( partialKey, realKey );
		}

		template< class T >
		inline void setImpl( const String& key, FieldType type, const T& value )
		{
			//generate name
			if( key.size() == 0 )
				map[ autoname() ] = new TypedEntry< T >( type, value );
			else
			{
				if( exists( key ) )
					SAFE_DELETE( map[key] );

				map[ key ] = new TypedEntry< T >( type, value );
			}
		}

		template< class T >
		inline void set( const String& key, FieldType type, const T& value )
		{			
			String actualKey;
			Table* t = getParentTable( key, actualKey );
			DEBUG_ASSERT( t != nullptr, "Cannot add a key to a non-existing table" );

			//actually set the key on the right table
			t->setImpl( actualKey, type, value );
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
		inline void set( const String& key, void* value, int size, bool managed = false )
		{
			DEBUG_ASSERT( value, "Setting a NULL Data value" );
			DEBUG_ASSERT( size >= 0, "Setting a Data value size <= 0" );

			set(key, FT_DATA, Data( value, size ) );
		}
		
		inline void set( const Table& value )
		{						
			set( value.getName(), FT_TABLE, value );
		}		
		
		///creates a new nested table named key
		/** 
		nested Tables always have name == key */
		inline Table* createTable( const String& key = String::EMPTY )
		{	
			String name;
			
			if( key.size() == 0 )
				name = autoname();
			else
				name = key;
							
			set( Table( name ) ); //always retain created tables
			
			return getTable( name ); //TODO don't do another search
		}
		
		///empties the map and deletes every value
		void clear()
		{					
			unnamedMembers = 0;
			
			//clean up every entry
			for( auto entry : map )
				SAFE_DELETE( entry.second );
			
			map.clear();
		}		
		
		///Inherits all the member in table t
		/** 
		After the call, this Table contains a copy of all the field defined in t but not here. 
		Nested Tables are an exception as if they're defined in both, the local nested table will
		recursively inherit the other nested table.
		*/
		void inherit( Table* t )
		{
			DEBUG_ASSERT( t != nullptr, "Cannot inherit a null Table" );

			//for each map member of the other map
			EntryMap::iterator itr = t->map.begin(),
								end = t->map.end(),
								existing;
			for( ; itr != end; ++itr )
			{
				existing = map.find( itr->first ); //look for a local element with the same name

				//element exists - do nothing except if it's a table
				if( existing != map.end() )
				{
					//if it's a table in both tables, inherit
					if( itr->second->type == FT_TABLE && existing->second->type == FT_TABLE )
						((Table*)existing->second->getValue())->inherit( (Table*)itr->second->getValue() );
				}
				else //just clone
					map[ itr->first ] = itr->second->clone();
			}
		}

		///total number of entries
		inline int size()
		{
			return (int)map.size();
		}
		
		inline const String& getName() const
		{
			return name;
		}

		///returns the total number of unnamed members
		inline int getAutoMembers() const
		{
			return unnamedMembers;
		}

		inline bool isEmpty()
		{
			return map.empty();
		}

		inline bool hasName() const
		{
			return name.size() > 0;
		}

		///returns true if this Table contains key
		inline bool exists( const String& key ) const
		{
			DEBUG_ASSERT( key.size(), "exists: key is empty" );

			return map.find( key ) != map.end();
		}

		///returns true if this Table contains key and the value is of type t
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
		
		///generic get
		inline Entry* get( const String& key ) const
		{
			String actualKey;
			const Table* container = getParentTable( key, actualKey );
			
			if( !container )
				return NULL;

			auto elem = container->map.find( actualKey );
			return elem != container->map.end() ? elem->second : NULL;
		}
		
		inline float getNumber( const String& key, float defaultValue = 0 ) const
		{			
			Entry* e = get( key );
			if( e && e->type == FT_NUMBER )
				return *( (float*)e->getValue() );
			else
				return defaultValue;
		}
		
		inline int getInt( const String& key, int defaultValue = 0 ) const
		{
			return (int)getNumber(key , (float)defaultValue);
		}
		
		inline bool getBool( const String& key, bool defaultValue = false ) const
		{
			Entry* e = get( key );
			if( e && e->type == FT_NUMBER )
				return (*( (float*)e->getValue() )) > 0;
			else
				return defaultValue;
		}
		
		inline const String& getString( const String& key, const String& defaultValue = String::EMPTY ) const
		{
			Entry* e = get( key );
			if( e && e->type == FT_STRING )
				return *( (String*)e->getValue() );
			else
				return defaultValue;
		}
		
		inline const Dojo::Vector& getVector( const String& key, const Dojo::Vector& defaultValue = Vector::ZERO ) const
		{
			Entry* e = get( key );
			if( e && e->type == FT_VECTOR )
				return *( (Vector*)e->getValue() );
			else
				return defaultValue;
		}
		
		inline const Dojo::Color getColor( const String& key, float alpha = 1.f, const Dojo::Color& defaultValue = Color::BLACK ) const
		{
			Entry* e = get( key );
			if( e && e->type == FT_VECTOR )
				return Color( *( (Vector*)e->getValue() ), alpha );
			else
				return defaultValue;
		}
		
		inline Table* getTable( const String& key ) const
		{			
			Entry* e = get( key );
			if( e && e->type == FT_TABLE )
				return (Table*)e->getValue();
			else
				return &EMPTY_TABLE;
		}
		
		inline const Data& getData( const String& key ) const
		{
			Entry* e = get( key );
			if( e && e->type == FT_DATA )
				return *( (Data*)e->getValue() );
			else
				return EMPTY_DATA;
		}	
		
		inline String autoMemberName( int idx ) const 
		{
			DEBUG_ASSERT( idx >= 0, "autoMemberName: idx is negative" );
			DEBUG_ASSERT_INFO( idx < getAutoMembers(), "autoMemberName: idx is OOB", String("idx = ") + idx );
			
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

		///returns a new unique anoymous id for a new "array member"
		inline String autoname()
		{
			return '_' + String( unnamedMembers++ );
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
		
		///write the table in string form over buf
		void serialize( String& buf, String indent = String::EMPTY ) const;

		void deserialize( StringReader& buf );
		
		///diagnostic method that serializes the table in a string
		inline String toString() const
		{
			String str = getName() + '\n';
			serialize( str );
			
			return str;
		}
		
		inline void debugPrint() const
		{
#ifdef _DEBUG			
			DEBUG_MESSAGE( toString().ASCII() );
#endif
		}
				
	protected:
		
		String name;
		
		EntryMap map;

		uint unnamedMembers;
	};
}


#endif
