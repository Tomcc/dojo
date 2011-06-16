#include "stdafx.h"

#include "Font.h"
#include "Platform.h"
#include "Table.h"
#include "FontSystem.h"

using namespace Dojo;

inline void blit( Dojo::byte* dest, Dojo::byte* src, uint x, uint y, uint destside, uint srcside )
{
	DEBUG_ASSERT( dest );
	DEBUG_ASSERT( src );

	uint rowy, py, idx;
	for( uint i = 0; i < srcside; ++i )
	{
		rowy = ( i + y ) * destside;

		for( uint j = 0; j < srcside; ++j )
		{
			idx = 4*(j+x) + rowy;

			dest[ idx ] = 0xff;     //color is always white
			dest[ idx + 1 ] = 0xff;
			dest[ idx + 2 ] = 0xff;
			dest[ idx + 3 ] = *src++;  //the font is really the alpha
		}
	}
}

Font::Page::Page( Font* f, int idx ) :
index( idx ),
firstCharIdx( index * FONT_CHARS_PER_PAGE ),
font( f )
{
	DEBUG_ASSERT( font );

	//notify me to each char
	for( uint i = 0; i < FONT_CHARS_PER_PAGE; ++i )
		chars[i].page = this;

	//create the texture
	uint sx = font->getFontWidth() * FONT_PAGE_SIDE;
	uint sy = font->getFontHeight() * FONT_PAGE_SIDE;

	uint sxp2 = Math::nextPowerOfTwo( sx );
	uint syp2 = Math::nextPowerOfTwo( sy );

	byte* buf = (byte*)malloc( sxp2 * syp2 * 4 );

	//render into buffer
	FT_Int32 renderMode = ( font->isAntialiased() ) ? FT_LOAD_RENDER : FT_LOAD_DEFAULT;

	font->_prepareFace();

	FT_GlyphSlot slot = font->face->glyph;

	unichar c = firstCharIdx;
	for( uint i = 0; i < FONT_PAGE_SIDE; ++i )
	{
		for( uint j = 0; j < FONT_PAGE_SIDE; ++j, ++c )
		{
			int err = FT_Load_Char( font->face, c, renderMode );
			DEBUG_ASSERT( err == 0 );

			//load character data

			//blit the bitmap if it was rendered
			if( font->face->glyph->bitmap.buffer )
				blit( 
					buf, 
					font->face->glyph->bitmap.buffer, 
					j * font->getFontWidth(),
					i * font->getFontHeight(),
					sxp2,
					font->face->glyph->bitmap.pitch);
		}
	}

	//drop the buffer in the texture
	texture = new Texture( NULL, "" );
	texture->loadFromMemory( buf, sxp2, syp2 );

	free( buf );
}

/// --------------------------------------------------------------------------------

/// --------------------------------------------------------------------------------

Font::Font( const std::string& path )
{			
	//miss all the pages
	memset( pages, 0, sizeof( void* ) * FONT_MAX_PAGES );

	//load table
	Table* t = new Table();	
	Platform::getSingleton()->load( t, path );

	fontFile = Utils::getDirectory( path ) + "/" + t->getString( "truetype" );
	fontWidth = fontHeight = t->getInt( "size" );
	border = t->exists( "border_color" );
	borderColor = t->getColor( "border_color" );
	antialias = t->getBool( "antialias" );

	face = Platform::getSingleton()->getFontSystem()->getFace( fontFile );

	Table* preload = t->getTable( "preloadedPages" );
	for( uint i = 0; i < preload->getAutoMembers(); ++i )
		getPage( preload->getInt( "_" + Utils::toString(i) ) );

}

void Font::_prepareFace()
{
	//set dimensions
	FT_Set_Pixel_Sizes(
		face,
		fontWidth,
		fontHeight );
}