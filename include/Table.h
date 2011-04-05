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

		typedef std::map< std::string, float > NumberMap;
		typedef std::map< std::string, std::string > StringMap;
		typedef std::map< std::string, Vector > VectorMap;
		typedef std::map< std::string, Data > DataMap;
		typedef std::map< std::string, Table* > TableMap;

		static const std::string UNDEFINED_STRING;
		static Table EMPTY_TABLE;
		static const Data EMPTY_DATA;
		
		Table( const std::string& tablename ) :
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
		
		inline void setNumber( const std::string& key, float value )
		{
			DEBUG_ASSERT( key.size() );
			
			numbers[key] = value;
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
			DEBUG_ASSERT( key.size() );
			
			strings[ key ] = value;
		}

		inline void setVector( const std::string& key, const Dojo::Vector& value )
		{
			DEBUG_ASSERT( key.size() );

			vectors[ key ] = value;
		}

		///WARNING - Table DOES NOT ACQUIRE OWNERSHIP OF THE DATA and requires it to be preserved!
		inline void setData( const std::string& key, void* value, uint size )
		{
			DEBUG_ASSERT( value );
			DEBUG_ASSERT( size );

			data[ key ] = Data( value, size );
		}
		
		inline void setTable( Table* value )
		{
			DEBUG_ASSERT( value );
			
			//avoid mem leaks
			if( existsAsTable( value->getName() ) )
				delete tables[ value->getName() ];
			
			tables[ value->getName() ] = value;
		}

		
		
		void clear()
		{
			numbers.clear();
			strings.clear();
			vectors.clear();
			data.clear();
						
			//cancella tutte le tabelle figlie
			TableMap::iterator itr;
			for( itr = tables.begin(); itr != tables.end(); ++itr )
				delete itr->second;
			
			tables.clear();
		}
		
		
		inline const std::string& getName()
		{
			return name;
		}

		inline bool existsAsNumber( const std::string& name )
		{
			DEBUG_ASSERT( name.size() );

			return numbers.find( name ) != numbers.end();
		}

		inline bool existsAsString( const std::string& name )
		{
			DEBUG_ASSERT( name.size() );

			return strings.find( name ) != strings.end();
		}

		inline bool existsAsTable( const std::string& name )
		{
			DEBUG_ASSERT( name.size() );

			return tables.find( name ) != tables.end();
		}

		inline bool existsAsVector( const std::string& name )
		{
			DEBUG_ASSERT( name.size() );

			return vectors.find( name ) != vectors.end();
		}

		inline bool existsAsData( const std::string& name )
		{
			DEBUG_ASSERT( name.size() );

			return data.find( name ) != data.end();
		}
		
		inline float getNumber( const std::string& key )
		{			
			if( existsAsNumber( key ) )
				return numbers[key];
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
			if( existsAsString(key) )
			   return strings[key];
			else
			   return UNDEFINED_STRING;
		}

		inline const Dojo::Vector& getVector( const std::string& key )
		{
			if( existsAsVector( key ) )
				return vectors[key];
			else
				return Vector::ZERO;
		}
		
		inline Table* getTable( const std::string& key )
		{			
			if( existsAsTable(key) )
			   return tables[key];
		   else
			   return &EMPTY_TABLE;
		}

		inline const Data& getData( const std::string& key )
		{
			if( existsAsData( key ) )
				return data[ key ];
			else
				return EMPTY_DATA;
		}		

		inline bool isEmpty()
		{
			return (numbers.size() + strings.size() + vectors.size() + data.size() + tables.size() ) == 0;
		}
		
		///scrive la tabella in un formato standard su stringa che inizia a pos
		void serialize( std::ostream& buf);

		void deserialize( std::istream& buf );

		inline const TableMap& _getTables()
		{
			return tables;
		}
				
	protected:
		
		std::string name;
		
		NumberMap numbers;
		StringMap strings;
		VectorMap vectors;
		DataMap data;
		TableMap tables;	

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

