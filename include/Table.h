#ifndef Table_h__
#define Table_h__

#include "dojo_common_header.h"

#include "BaseObject.h"
#include "Vector.h"

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
			vectors.clear();
						
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

			return numbers.find( name ) != numbers.end();
		}

		inline bool existsAsVector( const std::string& name )
		{
			DEBUG_ASSERT( name.size() );

			return vectors.find( name ) != vectors.end();
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
			   return NULL;
		}
		
		
		///scrive la tabella in un formato standard su stringa che inizia a pos
		inline void serialize( std::ostream& buf)
		{			
			using namespace std;
			
			buf << "<TABLE> " << name << endl;
			
			map< string, float >::iterator ni = numbers.begin();
			map< string, string >::iterator si = strings.begin();
			map< string, Vector >::iterator vi = vectors.begin();
			map< string, Table* >::iterator ti = tables.begin();
			
			if( numbers.size() )
				buf << "<NUMBERS>" << endl;
			
			for( ; ni != numbers.end(); ++ni )	
				buf << ni->first << '=' << ni->second << endl;
			
			if( strings.size() )
				buf << "<STRINGS>" << endl;
			
			for( ; si != strings.end(); ++si )	
				buf << si->first << '=' << si->second << endl;

			if( vectors.size() )
				buf << "<VECTORS>" << endl;
			
			for( ; vi != vectors.end(); ++vi )
				buf << vi->first << '=' << vi->second.x << ' ' << vi->second.y << ' ' << vi->second.z << endl;
			
			if( tables.size() )
				buf << "<TABLES>" << endl;
			
			for( ; ti != tables.end(); ++ti )	
				 ti->second->serialize( buf );
						
			buf << "<END>" << endl;
		}

		void deserialize( std::istream& buf )
		{
			std::string token = "<TABLE>", value;
			int numvalue;

			char line[ 200 ];

			while( token == "<TABLE>" && !buf.eof() )
				buf >> token;

			name = token;

			int state = 0;

			while( !buf.eof() )
			{
				_getLine( buf, line, 200, '=' );
				token.assign( line );

				if( token == "<NUMBERS>" )
					state = 0;
				else if( token == "<STRINGS>" )
					state = 1;
				else if( token == "<VECTORS>" )
					state = 2;
				else if( token == "<TABLES>" )
					state = 3;
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
						buf.getline( line, 200 );
						value.assign( line );
						setString( token, value );
					}
					else if( state == 2 )
					{
						Vector v;
						buf >> v.x;
						buf >> v.y;
						buf >> v.z;

						setVector( token, v );
					}
					else if( state == 2 )
					{						
						setTable( new Table( buf ) );
					}
				}
			}
		}
				
	protected:
		
		std::string name;
		
		std::map< std::string, float > numbers;
		std::map< std::string, std::string > strings;
		std::map< std::string, Vector > vectors;
		std::map< std::string, Table* > tables;	

		void _getLine( std::istream& in, char* buf, uint max, char delim )
		{
			char c='\n';
			uint i=0;

			while( c == '\n' )
				in.read( &c, 1 );

			while( !in.eof() && i < max )
			{
				if( c == delim || c == '\n' )
				{
					buf[i] = 0;
					return;
				}
				else
					buf[ i++ ] = c;

				in.read( &c, 1 );
			}
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

