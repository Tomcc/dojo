#include "stdafx.h"

#include "Font.h"

#include "Platform.h"

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
