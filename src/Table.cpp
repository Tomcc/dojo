#include "stdafx.h"

#include "Table.h"
#include "Platform.h"

using namespace Dojo;

#define EOF_CHAR 0

void Table::loadFromFile( Table* dest, const String& path )
{
	DEBUG_ASSERT( path.size() );

	FILE* file = fopen( path.ASCII().c_str(), "rb" );

	if( !file )
	{
		DEBUG_MESSAGE( "WARNING: Cannot load table " << path.ASCII() );
		return;
	}

	fseek( file, 0, SEEK_END);
	uint size = ftell (file);

	fseek( file, 0, SEEK_SET );

	std::string buf;
	buf.resize( size );

	fread( (void*)buf.data(), sizeof( unichar ), size, file );

	fclose( file );

	dest->setName( Utils::getFileName( path ) );

	StringReader reader( buf );
	dest->deserialize( reader );
}

Table Table::EMPTY_TABLE = Table( "EMPTY_TABLE" );

const Table::Data Table::EMPTY_DATA = Data(0,0);


bool Table::onLoad()
{
	//loads itself from file
	DEBUG_ASSERT( !loaded );

	if( !isReloadable() )
		return false;

	Platform::getSingleton()->load( this, filePath );

	return (loaded = !isEmpty());
}

void Table::serialize( String& buf, String indent ) const
{	
	using namespace std;	

	Data* data;
	Vector* v;
	
	//write 0xFFFE or they won't get this is unicode!
	buf += (unichar)0xfeff;

	//serialize to the Table Format	
	EntryMap::const_iterator itr = map.begin();
	
	for( ; itr != map.end(); ++itr ) 
	{
		Entry* e = itr->second;

		if( indent.size() )
			buf += indent;

		//write name and equal only if not anonymous and if not managed later
		if( itr->first[0] != '_' )
			buf += itr->first + " = ";

		switch( e->type )
		{
		case FT_NUMBER:
			buf.appendFloat( *((float*)e->getValue() ) );
			break;
		case FT_STRING:
			buf += '\"' + *((String*)e->getValue() ) + '\"';
			break;
		case FT_VECTOR:
			v = (Vector*)e->getValue();
			buf += '(';
			buf.appendFloat( v->x );
			buf += ' ';
			buf.appendFloat( v->y );
			buf += ' ';
			buf.appendFloat( v->z );
			buf += ')';

			break;
		case FT_DATA:
			data = (Data*)e->getValue();
			buf += '#' + String( data->size ) + ' ';

			buf.appendRaw( data->ptr, data->size );

			break;
		case FT_TABLE:
			buf += String( "{\n" );
			((Table*)e->getValue())->serialize( buf, indent + '\t' );

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
	PS_NAME_ENDED,
	PS_EQUAL,
	PS_COMMENT,
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
	PT_CHILD
};

inline bool isNameStarter( unichar c )
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

inline bool isNumber( unichar c )
{
	return (c >= '0' && c <= '9') || c == '-';  //- is part of a number!!!
}

inline bool isName( unichar c )
{
	return isNameStarter(c) || isNumber(c) || c == '_';
}

inline bool isValidFloat( unichar c )
{
	return isNumber( c ) || c == '.';
}

inline bool isWhiteSpace( unichar c )
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

void Table::deserialize( StringReader& buf )
{
	ParseState state = PS_TABLE;
	ParseTarget target = PT_UNDEFINED;
	
	String curName, str;
	float number;
	Vector vec;
	Data data;
	Table* table;
	Color col;

	//clear old
	clear();

	//feed one char at a time and do things
	unichar c = 1, c2;
	while( (!buf.eof() || c != EOF_CHAR ) && state != PS_END && state != PS_ERROR )
	{
		if( buf.eof() && c != EOF_CHAR )
			c = EOF_CHAR; //do check EOF_CHAR conditions
		else
			c = buf.get();
		
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
			else
				state = PS_NAME_ENDED;

			break;

		case PS_NAME_ENDED:
			if( c == '=' )
				state = PS_EQUAL;
			else if( isNameStarter( c ) || c == EOF_CHAR )
			{
				//compact bool!
				set( curName, (int)1 );
				state = PS_TABLE;
				buf.back(); //put back the read char because it belongs to another name
			}
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
			  
			//check if next char is x, that is, really we have an hex color!
			if( !buf.eof() )    c2 = buf.get();
			else                c2 = 0;
				
			if( c == '0' && c2 == 'x' )
			{
				buf.back();
				buf.back();
				
				DEBUG_MESSAGE( curName.ASCII() );
				
				//create a color using the hex
				col.set( buf.readHex() );
				
				set( curName, col );
			}
			else if( c == '-' && c2 == '-' ) //or, well, a comment! (LIKE A HACK)
			{
				//just skip until newline
				while( buf.get() != '\n' && !buf.eof() );
			}
			else
			{
				buf.back();
				buf.back();
					
				number = buf.readFloat();
					
				set( curName, number );
			}
				
			break;
		case PT_STRING:

			str.clear();
			while( true )
			{
				c = buf.get();
				if( c == '"' )	break;
				str += c;
			}

			set( curName, str );
			break;

		case PT_VECTOR:
			vec.x = buf.readFloat();
			vec.y = buf.readFloat();
			vec.z = buf.readFloat();

			set( curName, vec );
			break;

		case PT_DATA:
			data.size = (int)buf.readFloat();
			data.ptr = malloc( data.size );

			//skip space
			buf.get();

			buf.readBytes( data.ptr, data.size );

			set( curName, data.ptr, data.size, true ); //always retain deserialized data
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