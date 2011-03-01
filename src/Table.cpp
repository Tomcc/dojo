#include "stdafx.h"

#include "Table.h"

using namespace Dojo;

const std::string Table::UNDEFINED_STRING = "";
Table Table::EMPTY_TABLE = Table( "EMPTY_TABLE" );

void Table::serialize( std::ostream& buf)
{			
	using namespace std;

	buf << "<TABLE> " << name << endl;

	map< string, float >::iterator ni = numbers.begin();
	map< string, string >::iterator si = strings.begin();
	map< string, Vector >::iterator vi = vectors.begin();
	map< string, Data >::iterator di = data.begin();
	map< string, Table* >::iterator ti = tables.begin();

	if( numbers.size() )
	{
		buf << "<NUMBERS>" << endl;			
		for( ; ni != numbers.end(); ++ni )	
			buf << ni->first << '=' << ni->second << endl;
	}

	if( strings.size() )
	{
		buf << "<STRINGS>" << endl;			
		for( ; si != strings.end(); ++si )	
			buf << si->first << '=' << si->second << endl;
	}

	if( vectors.size() )
	{
		buf << "<VECTORS>" << endl;			
		for( ; vi != vectors.end(); ++vi )
			buf << vi->first << '=' << vi->second.x << ' ' << vi->second.y << endl;
	}

	if( data.size() )
	{
		buf << "<DATA>" << endl;
		for( ; di != data.end(); ++di )
		{
			buf << di->first << '=' << di->second.size << ' ';
			buf.write( (const char*)di->second.ptr, di->second.size );
			buf << endl;
		}					
	}

	if( tables.size() )
	{
		buf << "<TABLES>" << endl;

		for( ; ti != tables.end(); ++ti )	
			ti->second->serialize( buf );
	}

	buf << "<END>" << endl;
}

void Table::deserialize( std::istream& buf )
{
	std::string token = "<TABLE>", value;
	float numvalue;

	char line[ 200 ];

	enum ParseState
	{
		PS_NONE,
		PS_NUMBERS,
		PS_STRINGS,
		PS_VECTORS,
		PS_DATA,
		PS_TABLES,
		PS_END,
	};

	ParseState state = PS_NONE;

	while( !buf.eof() && state != PS_END )
	{
		_getLine( buf, line, 200, '=' );
		token.assign( line );

		if( token == "<NUMBERS>" )		state = PS_NUMBERS;
		else if( token == "<STRINGS>" )	state = PS_STRINGS;
		else if( token == "<VECTORS>" )	state = PS_VECTORS;
		else if( token == "<DATA>" )	state = PS_DATA;
		else if( token == "<TABLES>" )	state = PS_TABLES;
		else if( token == "<END>" )		state = PS_END;
		else 
		{					
			//numbers
			if( state == PS_NUMBERS )
			{
				buf >> numvalue;

				setNumber( token, numvalue );
			}					
			//strings
			else if( state == PS_STRINGS )
			{
				_getLine( buf, line, 200, '\n' );
				value.assign( line );
				setString( token, value );
			}
			else if( state == PS_VECTORS )
			{
				Vector v;
				buf >> v.x;
				buf >> v.y;

				setVector( token, v );
			}
			else if ( state == PS_DATA )
			{
				uint size;

				buf >> size;

				buf.ignore(); //skip the space

				void* data = malloc( size );
				buf.read( (char*)data, size );

				setData( token, data, size );
			}
			else if( state == PS_TABLES )
			{						
				token = token.substr( 8 );
				Table* t = createTable( token );
				t->deserialize( buf );
			}
		}
	}
}