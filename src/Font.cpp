#include "stdafx.h"

#include "Font.h"
#include "Platform.h"
#include "Table.h"
#include "FontSystem.h"

using namespace Dojo;

void blit( Dojo::byte* dest, FT_Bitmap* bitmap, uint x, uint y, uint destside )
{
	DEBUG_ASSERT( dest );
	DEBUG_ASSERT( bitmap );

	uint rowy, py, idx;
	if( bitmap->pixel_mode != FT_PIXEL_MODE_MONO )
	{
		for( uint i = 0; i < bitmap->rows; ++i )
		{
			rowy = ( i + y ) * destside;

			for( uint j = 0; j < bitmap->width; ++j )
			{
				idx = 4*( (j+x) + rowy );

				dest[ idx + 3 ] = bitmap->buffer[ j + i * bitmap->pitch ];  //the font is really the alpha
			}
		}
	}
	else
	{
		Dojo::byte b;
		for( uint i = 0; i < bitmap->rows; ++i )
		{
			rowy = ( i + y ) * destside;

			for( uint j = 0; j < bitmap->width; ++j )
			{
				idx = 4*( (j+x) + rowy );

				b = bitmap->buffer[ j/8 + i * bitmap->pitch ];
				b &= 1 << (7-(j%8));

				dest[ idx + 3 ] = (b > 0) ? 0xff : 0;  //the font is really the alpha
			}
		}
	}
}


Font::Page::Page( Font* f, int idx ) :
index( idx ),
firstCharIdx( index * FONT_CHARS_PER_PAGE ),
font( f )
{
	DEBUG_ASSERT( font );

	//create the texture
	uint sx = font->getFontWidth() * FONT_PAGE_SIDE;
	uint sy = font->getFontHeight() * FONT_PAGE_SIDE;

	uint sxp2 = Math::nextPowerOfTwo( sx );
	uint syp2 = Math::nextPowerOfTwo( sy );

	byte* buf = (byte*)malloc( sxp2 * syp2 * 4 );
	
	unsigned int * ptr = (unsigned int*)buf;
	//set alpha to 0
	for( int i = sxp2*syp2-1; i >= 0; --i )
		*ptr++ = 0x00ffffff;

	//render into buffer
	FT_Render_Mode renderMode = ( font->isAntialiased() ) ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO;

	font->_prepareFace();

	FT_GlyphSlot slot = font->face->glyph;

	unichar c = firstCharIdx;
	Font::Character* character = chars;
	uint x, y;

	for( uint i = 0; i < FONT_PAGE_SIDE; ++i )
	{
		for( uint j = 0; j < FONT_PAGE_SIDE; ++j, ++c, ++character )
		{
			int err = FT_Load_Char( font->face, c, FT_LOAD_DEFAULT );
			DEBUG_ASSERT( err == 0 );

			FT_Render_Glyph( font->face->glyph, renderMode );

			FT_Bitmap* bitmap = &(font->face->glyph->bitmap);

			x = j * font->getFontWidth();
			y = i * font->getFontHeight();

			//load character data
			character->page = this;
			character->character = c;
			character->pixelWidth = bitmap->width;
			character->uvWidth = (float)character->pixelWidth / (float)sxp2;
			character->uvHeight = (float)bitmap->rows / (float)syp2;
			character->uvPos.x = (float)x / (float)sxp2;
			character->uvPos.y = (float)y / (float)syp2;
			character->widthRatio = (float)character->pixelWidth / (float)font->getFontWidth();
			character->heightRatio = (float)bitmap->rows / (float)font->getFontHeight();

			//blit the bitmap if it was rendered
			if( bitmap->buffer )
				blit( 
					buf, 
					bitmap, 
					x,
					y,
					sxp2 );
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