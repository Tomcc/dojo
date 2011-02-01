#ifndef Table_h__
#define Table_h__

#include "dojo_common_header.h"

#include "BaseObject.h"

namespace Dojo
{
	class Table : public BaseObject 
	{
	public:
		
		Table( const std::string& tablename ) :
		name( tablename )
		{
			DEBUG_ASSERT( name.size() );
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
		
		inline void setString( const std::string& key, const std::string& value )
		{
			DEBUG_ASSERT( key.size() );
			
			strings[ key ] = value;
		}
		
		inline void setTable( Table* value )
		{
			DEBUG_ASSERT( value );
			
			Table* old = getTable( value->getName() );
			if( old )
				delete old;  //evita mem leaks
			
			tables[ value->getName() ] = value;
		}
		
		void clear()
		{
			numbers.clear();
			strings.clear();
						
			//cancella tutte le tabelle figlie
			std::map< std::string, Table* >::iterator itr;
			for( itr = tables.begin(); itr != tables.end(); ++itr )
				delete itr->second;
			
			tables.clear();
		}
		
		
		inline const std::string& getName()
		{
			return name;
		}
		
		inline float getNumber( const std::string& key )
		{			
			if( existsAsNumber( key ) )
				return numbers[key];
			else
				return 0;
		}
		
		inline const std::string& getString( const std::string& key )
		{			
			if( existsAsString(key) )
			   return strings[key];
			else
			   return "";
		}
		
		inline Table* getTable( const std::string& key )
		{			
			if( existsAsTable(key) )
			   return tables[key];
		   else
			   return NULL;
		}
		
		inline bool existsAsNumber( const std::string& name )
		{
			DEBUG_ASSERT( key.size() );
			
			return numbers.find( name ) != numbers.end();
		}
		
		inline bool existsAsString( const std::string& name )
		{
			DEBUG_ASSERT( key.size() );
			
			return numbers.find( name ) != numbers.end();
		}
		
		inline bool existsAsTable( const std::string& name )
		{
			DEBUG_ASSERT( key.size() );
			
			return numbers.find( name ) != numbers.end();
		}
		
		inline bool exists( const std::string& name )
		{
			return existsAsNumber(name) || existsAsString( name ) || existsAsTable( name );
		}
			   
	protected:
		
		std::string name;
		
		std::map< std::string, float > numbers;
		std::map< std::string, std::string > strings;
		std::map< std::string, Table* > tables;		
	};
}

#endif/*
 *  Table.h
 *  Drafted
 *
 *  Created by Tommaso Checchi on 2/1/11.
 *  Copyright 2011 none. All rights reserved.
 *
 */

