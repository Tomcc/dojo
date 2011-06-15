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
			FT_NUMBER,
			FT_STRING,
			FT_DATA,
			FT_VECTOR,
			FT_TABLE
		};

		struct Data
		{
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

		struct Entry 
		{
			FieldType type;
			void* value;

			Entry( FieldType fieldType, void* v ) :
			type( fieldType ),
			value( v )
			{
				
			}

			///need to call this before overwriting the entry
			void dispose()
			{
				if( type != FT_NUMBER )
					delete value;
			}
		};

		typedef std::map< std::string, Entry > EntryMap;

		static const std::string UNDEFINED_STRING;
		static Table EMPTY_TABLE;
		static const Data EMPTY_DATA;
		
		Table( const std::string& tablename = "" ) :
		name( tablename )
		{
			DEBUG_ASSERT( name.size() );
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

		inline void setRaw( const std::string& key, FieldType type, void* value )
		{
			DEBUG_ASSERT( key.size() );
			DEBUG_ASSERT( type == FT_NUMBER || (value != NULL) );

			if( exists( key ) )
				map[ key ].dispose();

			map[ key ] = Entry( type, value );
		}
		
		inline void setNumber( const std::string& key, float value )
		{			
			setRaw(key, FT_NUMBER, reinterpret_cast< float >( value ) );
		}

		inline void setInteger( const std::string& key, int value )
		{
			setNumber( key, (int)value );
		}

		inline void setBoolean( const std::string& key, bool value )
		{
			setNumber( key, (int)value );
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
			EntryMap::iterator itr;
			for( itr = map.begin(); itr != map.end(); ++itr )
				itr->second.dispose();
			
			map.clear();
		}		
		
		inline const std::string& getName()
		{
			return name;
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
				return reinterpret_cast< float >( map[key].value );
			else
				return 0;
		}

		/*inline const Entry& get( int i )
		{
			DEBUG_ASSERT( i < map.size() );

			return (map.begin() + i)->second;
		}*/

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

		inline const EntryMap::iterator getIterator()
		{
			return map.begin();
		}

		inline bool isEmpty()
		{
			return map.size() == 0;
		}
		
		///scrive la tabella in un formato standard su stringa che inizia a pos
		void serialize( std::ostream& buf);

		void deserialize( std::istream& buf );
				
	protected:
		
		std::string name;
		
		EntryMap map;

		void _getLine( std::istream& in, char* buf, uint max, char delim )
		{
			char c='\n';
			uint i=0;

			while( c == '\n' || c == '\r' )
				in.read( &c, 1 );

			while( !in.eof() && i < max )
			{
				if( c == delim || c == '\n' || c == '\r' )
				{
					buf[i] = 0;
					return;
				}
				else
					buf[ i++ ] = c;

				in.read( &c, 1 );
			}

			if( i < max )
				buf[i] = 0;
		}
	};
}


#endif

/*
 *  Table.h
 *  Drafted
 *
 *  Created by Tommaso Checchi on 2/1/11.
 *  Copyright 2011 none. All rights reserved.
 *
 */

