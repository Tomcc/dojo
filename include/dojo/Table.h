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
	class Entry;

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
			static const Data EMPTY;

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
			Table::FieldType type;

			Entry(Table::FieldType fieldType) :
				type(fieldType)
			{

			}

			virtual ~Entry()
			{

			}

			///returns a raw unyped pointer to the underlying data
			virtual void* getRawValue() = 0;

			float getAsNumber()
			{
				DEBUG_ASSERT(type == Table::FT_NUMBER, "type mismatch while reading from a Table Entry");
				return *(float*)getRawValue();
			}

			const String& getAsString()
			{
				DEBUG_ASSERT(type == Table::FT_STRING, "type mismatch while reading from a Table Entry");
				return *(String*)getRawValue();
			}

			const Vector& getAsVector()
			{
				DEBUG_ASSERT(type == Table::FT_VECTOR, "type mismatch while reading from a Table Entry");
				return *(Vector*)getRawValue();
			}

			Table& getAsTable()
			{
				DEBUG_ASSERT(type == Table::FT_TABLE, "type mismatch while reading from a Table Entry");
				return *(Table*)getRawValue();
			}

			const Table::Data& getAsData()
			{
				DEBUG_ASSERT(type == Table::FT_DATA, "type mismatch while reading from a Table Entry");
				return *(Table::Data*)getRawValue();
			}

			virtual Unique<Entry> clone() = 0;
		};

		template <class T>
		class TypedEntry : public Entry
		{
		public:

			T value;

			TypedEntry(FieldType fieldType, const T& v) :
				Entry(fieldType),
				value(v)
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

			virtual Unique<Entry> clone()
			{
				return make_unique< TypedEntry<T> >(type, value);
			}
		};

		typedef std::unordered_map< String, Unique<Entry> > EntryMap;

		static const Table EMPTY;
		
		static String index( int i )
		{
			return '_' + String(i);
		}

		///loads the file at path
		static Table loadFromFile( const String& path );
		
		///Creates a new table
		Table();

		Table(Table&& t);

		Table& operator=(Table&& t);

		Table(const Table& t);

		Table& operator=(const Table&) = delete;

		///Constructs a new "Table Resource", or a table bound to a file path in a ResourceGroup
		Table( ResourceGroup* creator, const String& path );

		~Table();

		virtual bool onLoad();

		virtual void onUnload( bool soft = false );

		///returns the table which contains the given "dot formatted" key
		/** it returns "this" for a normal non-hierarchical key
		returns "A" for a key such as "A.key"
		returns "B" for a key such as "A.B.key" */
		Table* getParentTable( const String& key, String& realKey ) const;

		template< class T >
		void setImpl( const String& key, FieldType type, const T& value )
		{
			map[ key.empty() ? autoname() : key ] = make_unique< TypedEntry< T > >( type, value );
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
		Table& createTable( const String& key = String::EMPTY );

		///empties the map and deletes every value
		void clear();		
		
		///Inherits all the member in table t
		/** 
		After the call, this Table contains a copy of all the field defined in t but not here. 
		Nested Tables are an exception as if they're defined in both, the local nested table will
		recursively inherit the other nested table.
		*/
		void inherit( Table* t );

		///total number of entries
		int size() const
		{
			return (int)map.size();
		}
		
		///returns the total number of unnamed members
		int getArrayLength() const
		{
			return unnamedMembers;
		}

		bool isEmpty() const
		{
			return map.empty();
		}
		
		operator bool() const {
			return !map.empty();
		}

		///returns true if this Table contains key
		bool exists( const String& key ) const;

		///returns true if this Table contains key and the value is of type t
		bool existsAs( const String& key, FieldType t ) const;
		
		///generic get
		Entry* get( const String& key ) const;
		
		float getNumber( const String& key, float defaultValue = 0 ) const;
		
		int getInt( const String& key, int defaultValue = 0 ) const;
		
		bool getBool( const String& key, bool defaultValue = false ) const;
		
		const String& getString( const String& key, const String& defaultValue = String::EMPTY ) const;
		
		const Vector& getVector( const String& key, const Vector& defaultValue = Vector::ZERO ) const;
		
		const Color getColor( const String& key, const Color& defaultValue = Color::BLACK ) const;
		
		const Table& getTable( const String& key ) const;
		
		const Data& getData( const String& key ) const;	
		
		String autoMemberName( int idx ) const;
		
		float getNumber( int idx ) const;
		
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
		
		const Table& getTable( int idx ) const
		{			
			return getTable( autoMemberName(idx) );
		}
		
		const Data& getData( int idx ) const
		{
			return getData( autoMemberName( idx ) );
		}	

		///returns a new unique anoymous id for a new "array member"
		String autoname();

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
		void remove( const String& key );
		
		///removes the unnamed member index idx
		void remove( int idx );

		///write the table in string form over buf
		void serialize( String& buf, String indent = String::EMPTY ) const;

		void deserialize( StringReader& buf );
		
		///diagnostic method that serializes the table in a string
		String toString() const;
		
		void debugPrint() const;

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


