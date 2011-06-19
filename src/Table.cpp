#include "stdafx.h"

#include "dojo/Table.h"

using namespace Dojo;

Table Table::EMPTY_TABLE = Table( "EMPTY_TABLE" );
const Table::Data Table::EMPTY_DATA = Data(0,0);

void Table::serialize( String& buf, String indent )
{	
	using namespace std;	

	Data* data;
	Vector* v;

	//serialize to the Table Format
	indent += '\t';
	
	for( Iterator itr = getIterator(); itr.valid(); ++itr ) 
	{
		const Entry& e = itr->second;

		if( indent.size() )
			buf += indent;

		//write name and equal only if not anonymous and if not managed later
		if( itr->first[0] != '_' )
			buf += itr->first + " = ";

		switch( e.type )
		{
		case FT_NUMBER:
			buf.appendFloat( *((float*)e.value) );
			break;
		case FT_STRING:
			buf += '\"' + *((String*)e.value) + '\"';
			break;
		case FT_VECTOR:
			v = (Vector*)e.value;
			buf += '(';
			buf.appendFloat( v->x );
			buf += ' ';
			buf.appendFloat( v->y );
			buf += ' ';
			buf.appendFloat( v->z );
			buf += ')';

			break;
		case FT_DATA:
			data = (Data*)e.value;
			buf += '#' + Utils::toString( data->size ) + ' ';

			buf.append( (unichar*)data->ptr, data->size/2 );

			break;
		case FT_TABLE:
			buf += String( "{\n" );
			((Table*)e.value)->serialize( buf, indent );

			buf += indent + '}';

			break;
		}

		buf += '\n';
	}
}

enum ParseState
{
	PS_TABLE,
	PS_NAME,
	PS_EQUAL,
	PS_END,
	PS_ERROR
};

enum ParseTarget
{
	PT_UNDEFINED,
	PT_NUMBER,
	PT_STRING,
	PT_DATA,
	PT_VECTOR,
	PT_CHILD,
};

inline bool isNameStarter( unichar c )
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

inline bool isNumber( unichar c )
{
	return c >= '0' && c <= '9';
}

inline bool isName( unichar c )
{
	return isNameStarter(c) || isNumber(c) || c == '_';
}

void Table::deserialize( InputStream& buf )
{
	ParseState state = PS_TABLE;
	ParseTarget target = PT_UNDEFINED;
	
	String curName, str;
	float number;
	Vector vec;
	Data data;
	Table* table;

	//clear old
	clear();

	//feed one char at a time and do things
	unichar c;
	char* bytes = (char*)&c;
	while( !buf.eof() && state != PS_END && state != PS_ERROR )
	{
		if( buf.tellg() % 2 == 1 ) //something did read only the first byte of something
			buf.get();

		bytes[0] = (char)buf.get();
		bytes[1] = (char)buf.get();

		switch( state )
		{
		case PS_TABLE: //wait for either a name, or an anon value	
				 if( c == '}' )				state = PS_END;
			else if( isNameStarter( c ) )	state = PS_NAME;

			else if( c == '"' )		target = PT_STRING;
			else if( c == '(' )		target = PT_VECTOR;			
			else if( c == '#' )		target = PT_DATA;
			else if( c == '{' )		target = PT_CHILD;
			else if( isNumber( c ) )target = PT_NUMBER;

			if( state == PS_NAME )
			{
				curName.clear();
				curName += c;
			}

			break;
		case PS_NAME:			
			if( isName( c ) )
				curName += c;
			else if( c == '=' )
				state = PS_EQUAL;

			break;

		case PS_EQUAL: //wait for value start
				 if( c == '"' )		target = PT_STRING;
			else if( c == '(' )		target = PT_VECTOR;			
			else if( c == '#' )		target = PT_DATA;
			else if( c == '{' )		target = PT_CHILD;
			else if( isNumber( c ) )target = PT_NUMBER;

			break;
		}

		switch( target )
		{
		case PT_NUMBER:
			buf.putback(c);
			buf >> number;

			setNumber( curName, number );
			break;
		case PT_STRING:

			str.clear();
			while( true )
			{
				bytes[0] = (char)buf.get();
				bytes[1] = (char)buf.get();

				if( c == '"' )	break;
				str += c;
			}

			setString( curName, str );
			break;

		case PT_VECTOR:
			buf >> vec.x;
			buf >> vec.y;
			buf >> vec.z;

			setVector( curName, vec );
			break;

		case PT_DATA:
			buf >> data.size;
			data.ptr = malloc( data.size );

			//skip space
			buf.get();

			buf.read( (unichar*)data.ptr, data.size );

			setData( curName, data.ptr, data.size );
			break;

		case PT_CHILD:
			table = createTable(curName);
			table->deserialize( buf );

			break;
		}

		if( target ) //read something
		{
			state = PS_TABLE;
			target = PT_UNDEFINED;
			curName.clear();
		}
	}
}