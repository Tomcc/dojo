#include "stdafx.h"

#include "Font.h"

#include "Platform.h"
#include "Table.h"

using namespace Dojo;

Font::Page::Page( const std::string& fontName, int idx ) :
index( idx ),
firstCharIdx( index * FONT_CHARS_PER_PAGE )
{
	//notify me to each char
	for( uint i = 0; i < FONT_CHARS_PER_PAGE; ++i )
		chars[i].page = this;

	texture = _createTextureFromTTF( fontName );

	//load each char
	DEBUG_TODO;
}

Texture* Font::Page::_createTextureFromTTF( const std::string& name )
{
	//create the texture somehow
	DEBUG_TODO;

	return NULL;
}

Font::Font( const std::string& path )
{			
	//miss all the pages
	memset( pages, 0, sizeof( void* ) * FONT_MAX_PAGES );

	//load table
	Table* t = new Table();	
	Platform::getSingleton()->load( t, path );

	fontFile = t->getString( "truetype" );
	fontHeight = t->getInt( "size" );
	border = t->exists( "border_color" );
	borderColor = t->getColor( "border_color" );

	Table* preload = t->getTable( "preloadedPages" );
	for( uint i = 0; i < preload->getAutoMembers(); ++i )
		getPage( preload->getInt( "_" + Utils::toString(i) ) );

	Platform::getSingleton()->save( t, path.substr(0, path.find_last_of('/') ) + "out.ds" );
}