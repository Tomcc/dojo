#include "stdafx.h"

#include "Table.h"
#include "Platform.h"
#include "FileStream.h"

using namespace Dojo;

void Table::loadFromFile( Table* dest, const String& path )
{
	DEBUG_ASSERT( dest != nullptr, "The destination table is null" );
	DEBUG_ASSERT( path.size(), "Tried to load a Table from an empty path string" );

    auto file = Platform::getSingleton()->getFile( path );
	
	if( file->open() )
	{
		//read the contents directly in a string
		std::string buf;
		buf.resize( file->getSize() );

		file->read( (byte*)buf.c_str(), buf.size() );

		dest->setName( Utils::getFileName( path ) );

		StringReader reader( buf );
		dest->deserialize( reader );
	}
}

Table Table::EMPTY_TABLE = Table( "EMPTY_TABLE" );

const Table::Data Table::EMPTY_DATA = Data(0,0);


bool Table::onLoad()
{
	//loads itself from file
	DEBUG_ASSERT( !isLoaded(), "The Table is already loaded" );

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
            default: break;
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
	PT_CHILD,
	PT_IMPLICITTRUE
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
	while( state != PS_END && state != PS_ERROR )
	{
		c = buf.get();
		
		switch( state )
		{
		case PS_TABLE: //wait for either a name, or an anon value	
				 if( c == '}' || c == 0 )				state = PS_END;
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

		case PS_NAME_ENDED:  //wait for an equal; drop whitespace and fall back if other is found
			if( c == '=' )
				state = PS_EQUAL;
			else if( !isWhiteSpace(c) ) //it is something else - store this as an implicit bool and reset the parser
				target = PT_IMPLICITTRUE;

			break;

		case PS_EQUAL: //wait for value start
				 if( c == '"' )		target = PT_STRING;
			else if( c == '(' )		target = PT_VECTOR;			
			else if( c == '#' )		target = PT_DATA;
			else if( c == '{' )		target = PT_CHILD;
			else if( isNumber( c ) )target = PT_NUMBER;

			break;
        default: ;
		}

		switch( target )
		{
		case PT_IMPLICITTRUE:
			buf.back();
			set( curName, (int)1 );
			break;

		case PT_NUMBER:
			  
			//check if next char is x, that is, really we have an hex color!
			c2 = buf.get();
				
			if( c == '0' && c2 == 'x' )
			{
				buf.back();
				buf.back();
				
				//create a color using the hex
				col.set( buf.readHex() );
				
				set( curName, col );
			}
			else if( c == '-' && c2 == '-' ) //or, well, a comment! (LIKE A HACK)
			{
				//just skip until newline
				do { c = buf.get(); } while( c != 0 && c != '\n' );
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
			while( 1 )
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
        default: ;
		}

		if( target ) //read something
		{
			state = PS_TABLE;
			target = PT_UNDEFINED;
			curName.clear();
		}
	}
}