#pragma once

#include "dojo_common_header.h"

#include "Vector.h"
#include "Array.h"
#include "Utils.h"
#include "StringReader.h"
#include "dojostring.h"
#include "Resource.h"
#include "Log.h"

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
			int size;
			
			Data() :
			ptr( NULL ),
			size( 0 )
			{

			}

			Data( void* p, int s) :
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
								
			///returns a raw unyped pointer to the underlying data
			virtual void* getRawValue() =0;

			float getAsNumber() 
			{
				DEBUG_ASSERT( type == FT_NUMBER, "type mismatch while reading from a Table Entry" );
				return *(float*)getRawValue();
			}

			const String& getAsString()
			{
				DEBUG_ASSERT( type == FT_STRING, "type mismatch while reading from a Table Entry" );
				return *(String*)getRawValue();
			}

			const Vector& getAsVector()
			{
				DEBUG_ASSERT( type == FT_VECTOR, "type mismatch while reading from a Table Entry" );
				return *(Vector*)getRawValue();
			}

			Table* getAsTable()
			{
				DEBUG_ASSERT( type == FT_TABLE, "type mismatch while reading from a Table Entry" );
				return (Table*)getRawValue();
			}

			const Data& getAsData()
			{
				DEBUG_ASSERT( type == FT_DATA, "type mismatch while reading from a Table Entry" );
				return *(Data*)getRawValue();
			}

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
			
			///returns the raw value pointer
			virtual void* getRawValue()
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
		
		static String index( int i )
		{
			return '_' + String(i);
		}

		///loads the file at path
		static Table loadFromFile( const String& path );
		
		///Creates a new table
		Table( const String& tablename = String::EMPTY ) :
		Resource(),
		unnamedMembers( 0 )
		{

		}

		Table(Table&& t) :
			Resource(),
			unnamedMembers(t.unnamedMembers),
			map(std::move(t.map))
		{
			t.unnamedMembers = 0;
		}

		Table& operator=(Table&& t) 
		{
			unnamedMembers = t.unnamedMembers;
			map = std::move(t.map);
			return *this;
		}

		///Constructs a new "Table Resource", or a table bound to a file path in a ResourceGroup
		Table( ResourceGroup* creator, const String& path ) :
		Resource( creator, path ),
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

		///returns the table which contains the given "dot formatted" key
		/** it returns "this" for a normal non-hierarchical key
		returns "A" for a key such as "A.key"
		returns "B" for a key such as "A.B.key" */
		Table* getParentTable( const String& key, String& realKey ) const
		{
			size_t dotIdx = 0;
			for(; dotIdx< key.size() && key[dotIdx] != '.'; ++dotIdx ); 

			if( dotIdx == key.size() )
			{
				realKey = key;
				return (Table*)this;
			}

			String partialKey = key.substr( dotIdx+1 );
			String childName = key.substr( 0, dotIdx );
			auto& child = getTable( childName );

			DEBUG_ASSERT_INFO( child.size() > 0, "A part of a dot-formatted key referred to a non-existing table", "childName = " + childName );

			return child.getParentTable( partialKey, realKey );
		}

		template< class T >
		void setImpl( const String& key, FieldType type, const T& value )
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
		void set( const String& key, FieldType type, const T& value )
		{			
			String actualKey;
			Table* t = getParentTable( key, actualKey );
			DEBUG_ASSERT( t != nullptr, "Cannot add a key to a non-existing table" );

			//actually set the key on the right table
			t->setImpl( actualKey, type, value );
		}
		
		void set( const String& key, float value )
		{			
			set(key, FT_NUMBER, value );
		}

		void set( const String& key, int value )
		{
			set( key, (float)value );
		}

		///boolean has to be specified as C has the ugly habit of casting everything to it without complaining
		void setBoolean( const String& key, bool value )
		{
			set( key, (float)value );
		}
		
		void set( const String& key, const String& value )
		{			
			set(key, FT_STRING, value );
		}

		void set( const String& key, const Vector& value )
		{
			set( key, FT_VECTOR, value );
		}
		
		void set( const String& key, const Color& value )
		{
			set( key, FT_VECTOR, Vector( value.r, value.g, value.b ) );
		}

		///WARNING - Data DOES NOT ACQUIRE OWNERSHIP OF THE DATA!!!
		void set( const String& key, void* value, int size, bool managed = false )
		{
			DEBUG_ASSERT( value, "Setting a NULL Data value" );
			DEBUG_ASSERT( size >= 0, "Setting a Data value size <= 0" );

			set(key, FT_DATA, Data( value, size ) );
		}
		
		void set( const String& key, const Table& value )
		{						
			set( key, FT_TABLE, value );
		}		
		
		///creates a new nested table named key
		/** 
		nested Tables always have name == key */
		Table& createTable( const String& key = String::EMPTY )
		{	
			String name;
			
			if( key.size() == 0 )
				name = autoname();
			else
				name = key;
							
			set( key, Table( name ) ); //always retain created tables
			
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
						((Table*)existing->second->getRawValue())->inherit( (Table*)itr->second->getRawValue() );
				}
				else //just clone
					map[ itr->first ] = itr->second->clone();
			}
		}

		///total number of entries
		int size()
		{
			return (int)map.size();
		}
		
		///returns the total number of unnamed members
		int getAutoMembers() const
		{
			return unnamedMembers;
		}

		bool isEmpty()
		{
			return map.empty();
		}

		///returns true if this Table contains key
		bool exists( const String& key ) const
		{
			DEBUG_ASSERT( key.size(), "exists: key is empty" );

			return map.find( key ) != map.end();
		}

		///returns true if this Table contains key and the value is of type t
		bool existsAs( const String& key, FieldType t ) const
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
		Entry* get( const String& key ) const
		{
			String actualKey;
			const Table* container = getParentTable( key, actualKey );
			
			if( !container )
				return NULL;

			auto elem = container->map.find( actualKey );
			return elem != container->map.end() ? elem->second : NULL;
		}
		
		float getNumber( const String& key, float defaultValue = 0 ) const
		{			
			Entry* e = get( key );
			if( e && e->type == FT_NUMBER )
				return e->getAsNumber();
			else
				return defaultValue;
		}
		
		int getInt( const String& key, int defaultValue = 0 ) const
		{
			return (int)getNumber(key , (float)defaultValue);
		}
		
		bool getBool( const String& key, bool defaultValue = false ) const
		{
			Entry* e = get( key );
			if( e && e->type == FT_NUMBER )
				return e->getAsNumber() > 0;
			else
				return defaultValue;
		}
		
		const String& getString( const String& key, const String& defaultValue = String::EMPTY ) const
		{
			Entry* e = get( key );
			if( e && e->type == FT_STRING )
				return e->getAsString();
			else
				return defaultValue;
		}
		
		const Vector& getVector( const String& key, const Vector& defaultValue = Vector::ZERO ) const
		{
			Entry* e = get( key );
			if( e && e->type == FT_VECTOR )
				return e->getAsVector();
			else
				return defaultValue;
		}
		
		const Color getColor( const String& key, float alpha = 1.f, const Color& defaultValue = Color::BLACK ) const
		{
			Entry* e = get( key );
			if( e && e->type == FT_VECTOR )
				return Color( e->getAsVector(), alpha );
			else
				return defaultValue;
		}
		
		Table& getTable( const String& key ) const
		{			
			Entry* e = get( key );
			if( e && e->type == FT_TABLE )
				return *e->getAsTable();
			else
				return EMPTY_TABLE;
		}
		
		const Data& getData( const String& key ) const
		{
			Entry* e = get( key );
			if( e && e->type == FT_DATA )
				return e->getAsData();
			else
				return EMPTY_DATA;
		}	
		
		String autoMemberName( int idx ) const 
		{
			DEBUG_ASSERT( idx >= 0, "autoMemberName: idx is negative" );
			DEBUG_ASSERT_INFO( idx < getAutoMembers(), "autoMemberName: idx is OOB", String("idx = ") + idx );
			
			return '_' + String( idx );
		}
		
		float getNumber( int idx ) const
		{			
			return getNumber( autoMemberName( idx ) );
		}
		
		int getInt( int idx ) const
		{
			return (int)getNumber( idx );
		}
		
		bool getBool( int idx ) const
		{
			return getNumber(idx) > 0.f;
		}
		
		const String& getString( int idx ) const
		{
			return getString( autoMemberName(idx) );
		}
		
		const Vector& getVector( int idx ) const
		{
			return  getVector( autoMemberName(idx ) );
		}
		
		const Color getColor( int idx, float alpha = 1.f ) const
		{
			return Color( getVector( idx ), alpha );
		}
		
		Table& getTable( int idx ) const
		{			
			return getTable( autoMemberName(idx) );
		}
		
		const Data& getData( int idx ) const
		{
			return getData( autoMemberName( idx ) );
		}	

		///returns a new unique anoymous id for a new "array member"
		String autoname()
		{
			return '_' + String( unnamedMembers++ );
		}

		template<typename T>
		void push(const T& t) {
			set(autoname(), t);
		}
		
		const EntryMap::const_iterator begin() const
		{
			return map.begin();
		}
		
		const EntryMap::const_iterator end() const
		{
			return map.end();
		}
		
		///removes a member named key
		void remove( const String& key )
		{
			map.erase( key );
		}
		
		///removes the unnamed member index idx
		void remove( int idx )
		{
			map.erase( autoMemberName( idx ) );
		}

		bool isEmpty() const
		{
			return map.empty();
		}
		
		///write the table in string form over buf
		void serialize( String& buf, String indent = String::EMPTY ) const;

		void deserialize( StringReader& buf );
		
		///diagnostic method that serializes the table in a string
		String toString() const
		{
			String str;
			serialize( str );
			
			return str;
		}
		
		void debugPrint() const
		{
#ifdef _DEBUG			
			DEBUG_MESSAGE( toString() );
#endif
		}

		///returns an iterator to the beginning of the internal dictionary
		EntryMap::iterator begin()
		{
			return map.begin();
		}

		///returns an iterator to the end of the internal dictionary
		EntryMap::iterator end()
		{
			return map.end();
		}
				
	protected:
		
		EntryMap map;

		int unnamedMembers;
	};
}


