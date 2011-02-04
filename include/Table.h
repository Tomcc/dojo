#ifndef Table_h__
#define Table_h__

#include "dojo_common_header.h"

#include "BaseObject.h"

namespace Dojo
{
	class Table : public BaseObject 
	{
	public:

		static const std::string UNDEFINED_STRING;
		
		Table( const std::string& tablename ) :
		name( tablename )
		{
			DEBUG_ASSERT( name.size() );
		}
		
		///legge la tabella dal formato standard su stringa
		Table( std::stringstream& buf )
		{						
			std::string token = "<TABLE>", value;
			int numvalue;
			
			while( token == "<TABLE>" )
				  buf >> token;
				  
			name = token;
			
			int state = 0;
			
			while( 1 )
			{
				buf >> token;				
				
				if( token == "<NUMBERS>" )
					state = 0;
				else if( token == "<STRINGS>" )
					state = 1;
				else if( token == "<TABLES>" )
					state = 2;
				else if( token == "<END>" )
					break;
				else 
				{					
					//numbers
					if( state == 0 )
					{
						buf >> numvalue;
						
						setNumber( token, numvalue );
					}					
					//strings
					else if( state == 1 )
					{
						buf >> value;
						setString( token, value );
					}
					else if( state == 2 )
					{						
						setTable( new Table( buf ) );
					}
				}
			}
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

		inline void setNumber( const std::string& key, int value )
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
		
		inline Table* getTable( const std::string& key )
		{			
			if( existsAsTable(key) )
			   return tables[key];
		   else
			   return NULL;
		}
		
		inline bool existsAsNumber( const std::string& name )
		{
			DEBUG_ASSERT( name.size() );
			
			return numbers.find( name ) != numbers.end();
		}
		
		inline bool existsAsString( const std::string& name )
		{
			DEBUG_ASSERT( name.size() );
			
			return numbers.find( name ) != numbers.end();
		}
		
		inline bool existsAsTable( const std::string& name )
		{
			DEBUG_ASSERT( name.size() );
			
			return numbers.find( name ) != numbers.end();
		}
		
		inline bool exists( const std::string& name )
		{
			return existsAsNumber(name) || existsAsString( name ) || existsAsTable( name );
		}
		
		///scrive la tabella in un formato standard su stringa che inizia a pos
		inline void serialize( std::stringstream& buf)
		{			
			buf << "<TABLE> " << name << std::endl;
			
			std::map< std::string, float >::iterator ni = numbers.begin();
			std::map< std::string, std::string >::iterator si = strings.begin();
			std::map< std::string, Table* >::iterator ti = tables.begin();
			
			buf << "<NUMBERS>" << std::endl;
			
			for( ; ni != numbers.end(); ++ni )	
				buf << ni->first << ' ' << ni->second << std::endl;
			
			buf << "<STRINGS>" << std::endl;
			
			for( ; si != strings.end(); ++si )	
				buf << si->first << ' ' << si->second << std::endl;
			
			buf << "<TABLES>" << std::endl;
			
			for( ; ti != tables.end(); ++ti )	
				 ti->second->serialize( buf );
						
			buf << "<END>" << std::endl;
		}
				
	protected:
		
		std::string name;
		
		std::map< std::string, float > numbers;
		std::map< std::string, std::string > strings;
		std::map< std::string, Table* > tables;	
		
	};

	const std::string Table::UNDEFINED_STRING = "";
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

