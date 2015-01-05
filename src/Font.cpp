#include "stdafx.h"

#include "Font.h"
#include "Platform.h"
#include "Table.h"
#include "FontSystem.h"
#include "Timer.h"
#include "ResourceGroup.h"
#include "Texture.h"
#include "FrameSet.h"
#include "Tessellation.h"
#include "Utils.h"

using namespace Dojo;


Font::Character::Character() {

}

Texture* Font::Character::getTexture() {
	return page->getTexture();
}

Tessellation* Font::Character::getTesselation() {
	DEBUG_ASSERT(mTesselation.get(), "Tried to get a tesselation but this Font wasn't tesselated (forgot to specify 'tesselate' in the config?)");

	return mTesselation.get();
}

void Font::_blit(byte* dest, FT_Bitmap* bitmap, int x, int y, int destside)
{
	DEBUG_ASSERT( dest, "null destination buffer" );
	DEBUG_ASSERT( bitmap, "Null freetype bitmap" );

	int rowy, idx;
	if( bitmap->pixel_mode != FT_PIXEL_MODE_MONO )
	{
		for( int i = 0; i < bitmap->rows; ++i )
		{
			rowy = ( i + y ) * destside;

			for( int j = 0; j < bitmap->width; ++j )
			{
				idx = 4*( (j+x) + rowy );

				dest[ idx + 3 ] = bitmap->buffer[ j + i * bitmap->pitch ];  //the font is really the alpha
			}
		}
	}
	else
	{
		byte b;
		for( int i = 0; i < bitmap->rows; ++i )
		{
			rowy = ( i + y ) * destside;

			for( int j = 0; j < bitmap->width; ++j )
			{
				idx = 4*( (j+x) + rowy );

				b = bitmap->buffer[ j/8 + i * bitmap->pitch ];
				b &= 1 << (7-(j%8));

				dest[ idx + 3 ] = (b > 0) ? 0xff : 0;  //the font is really the alpha
			}
		}
	}
}

void Font::_blitborder(byte* dest, FT_Bitmap* bitmap, int x, int y, int destside, const Color& col)
{
	DEBUG_ASSERT( dest, "null destination buffer" );
	DEBUG_ASSERT( bitmap, "Null freetype bitmap" );

	int rowy;
	byte* ptr;

	for( int i = 0; i < bitmap->rows; ++i )
	{
		rowy = ( i + y ) * destside;

		for( int j = 0; j < bitmap->width; ++j )
		{
			ptr = dest + 4*( (j+x) + rowy );

			float a = (float)bitmap->buffer[ j + i * bitmap->pitch ];
			a /= 255.f;
			float inva = 1.f - a;

			ptr[0] = (byte)(ptr[0] * inva + a * col.r * 255.f);
			ptr[1] = (byte)(ptr[1] * inva + a * col.g * 255.f);
			ptr[2] = (byte)(ptr[2] * inva + a * col.b * 255.f);
			ptr[3] = (byte)(ptr[3] * inva + a * a * 255.f);  //the font is really the alpha
		}
	}
}

float gCurrentScale;

int _moveTo( const FT_Vector* to, void* ptr )
{
	auto t = ((Font::Character*)ptr)->getTesselation();

	t->startPath( Vector( to->x * gCurrentScale, to->y * gCurrentScale ) );

	return 0;
}

int _lineTo( const FT_Vector* to, void* ptr )
{
	auto t = ((Font::Character*)ptr)->getTesselation();

	t->addSegment( Vector( to->x * gCurrentScale, to->y * gCurrentScale ) );
	return 0;
}

int _conicTo( const FT_Vector*  control, const FT_Vector*  to, void* ptr )
{
	auto t = ((Font::Character*)ptr)->getTesselation();

	t->addQuadradratic( 
		Vector( control->x * gCurrentScale, control->y * gCurrentScale), 
		Vector( to->x * gCurrentScale, to->y * gCurrentScale ), 
		((Font::Character*)ptr)->page->getFont()->getPolyOutlineQuality() );

	return 0;
}

int _cubicTo( const FT_Vector*  control1, const FT_Vector*  control2, const FT_Vector*  to,	 void* ptr )
{
	auto t = ((Font::Character*)ptr)->getTesselation();

	t->addCubic( 
		Vector( control1->x * gCurrentScale, control1->y * gCurrentScale ), 
		Vector( control2->x * gCurrentScale, control2->y * gCurrentScale ), 
		Vector( to->x * gCurrentScale, to->y * gCurrentScale ), 
		((Font::Character*)ptr)->page->getFont()->getPolyOutlineQuality() );

	return 0;
}

void Font::Character::init( Page* p, unichar c, int x, int y, int sx, int sy, FT_Glyph_Metrics* metrics, FT_Outline* outline )
{
	DEBUG_ASSERT( p, "Character needs a non-null parent Page" );
	DEBUG_ASSERT( metrics, "null FreeType font metrics" );

	float fsx = (float)sx;
	float fsy = (float)sy;
	float fw = (float)p->getFont()->getFontWidth();
	float fh = (float)p->getFont()->getFontHeight();
	Font* f = p->getFont();

	page = p;
	character = c;
	gliphIdx = f->getCharIndex( this );

	pixelWidth = page->getFont()->mCellWidth;
	float pixelHeight = (float)metrics->height * FONT_PPI + page->getFont()->glowRadius*2;

	uvWidth = (float)pixelWidth / fsx;
	uvHeight = pixelHeight / fsy;
	uvPos.x = (float)x / fsy;
	uvPos.y = (float)y / fsy;
	widthRatio = (float)pixelWidth / fw;
	heightRatio = pixelHeight / fh;

	bearingU = ((float)metrics->horiBearingX * FONT_PPI ) / fsy;
	bearingV = ((float)(metrics->height - metrics->horiBearingY) * FONT_PPI ) / fh;

	advance = ((float)metrics->horiAdvance * FONT_PPI ) / fw;

	if( page->getFont()->generateEdge || page->getFont()->generateSurface ) //tesselate ALL the things!
	{
		Timer timer;

		DEBUG_ASSERT( outline, "No outline provided but the font should be tesselated" );
		mTesselation = Unique< Tessellation >( new Tessellation() );

		//find the normalizing scale and call the tesselation functions
		gCurrentScale = (float)FONT_PPI / (float)fw;
		FT_Outline_Funcs funcs = { _moveTo, _lineTo, _conicTo, _cubicTo, 0,0 };

		FT_Outline_Decompose( outline, &funcs, this );

		//now that everything is loaded & in order, tessellate the mesh
		if (mTesselation->segments.size() && page->getFont()->generateSurface) { //HACK

			int options = Tessellation::PREPARE_EXTRUSION | Tessellation::GUESS_HOLES;
			if (!page->getFont()->generateEdge)
				options |= Tessellation::CLEAR_INPUTS;

			mTesselation->tessellate(options); //keep edges if they are needed too
		}
	}
}


Font::Page::Page( Font* f, int idx ) :
Resource(),
index( idx ),
firstCharIdx( index * FONT_CHARS_PER_PAGE ),
font( f ),
texture( NULL )
{
	DEBUG_ASSERT( font, "Page needs a non-null parent font" );

	texture = new Texture();
}

Font::Page::~Page() {
	SAFE_DELETE(texture);
}


bool Font::Page::onLoad()
{
	//create the texture
	int sx = font->mCellWidth * FONT_PAGE_SIDE;
	int sy = font->mCellHeight * FONT_PAGE_SIDE;

	bool npot = Platform::singleton().isNPOTEnabled();
	int sxp2 = npot ? sx : Math::nextPowerOfTwo( sx );
	int syp2 = npot ? sy : Math::nextPowerOfTwo( sy );

	int pixelNumber = sxp2 * syp2;
	int bufsize = pixelNumber * 4;
	byte* buf = (byte*)malloc( bufsize );

	unsigned int * ptr = (unsigned int*)buf;
	//set alpha to 0 and colours to white
	for( int i = sxp2*syp2-1; i >= 0; --i )
		*ptr++ = 0x00ffffff;

	//render into buffer
	FT_Render_Mode renderMode = ( font->isAntialiased() ) ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO;
	FT_GlyphSlot slot = font->face->glyph;
	FT_Bitmap* bitmap;
	int gliphidx;

	font->_prepareFace();

	unichar c = firstCharIdx;
	Font::Character* character = chars;
	int x, y;

	Timer timer;

	for (int i = 0; i < FONT_PAGE_SIDE; ++i)
	{
		for (int j = 0; j < FONT_PAGE_SIDE; ++j, ++c, ++character)
		{
			gliphidx = FT_Get_Char_Index( font->face, c );
			int err = FT_Load_Glyph( font->face, gliphidx, FT_LOAD_DEFAULT );

			x = j * font->mCellWidth;
			y = i * font->mCellHeight;

			//load character data
			character->init( 
				this,
				c, 
				x, y,
				sxp2, syp2, 
				&(slot->metrics),
				&slot->outline );

			FT_Render_Glyph( slot, renderMode );

			bitmap = &(slot->bitmap);

			//blit the bitmap if it was rendered
			if( bitmap->buffer )
				_blit( buf, bitmap, x + font->glowRadius, y + font->glowRadius, sxp2 );
		}
	}

	//glow?	
	if( font->glowRadius > 0 )
	{
		unsigned int glowCol = font->glowColor.toRGBA();
		byte* glowColChannel = (byte*)&glowCol;

		//duplicate the buffer
		byte* glowBuf = (byte*)malloc( bufsize );
		memcpy( glowBuf, buf, bufsize );

		for( int iteration = 0; iteration < font->glowRadius; ++iteration )
		{
			for( int i = 1; i < syp2-1; ++i )
			{
				for( int j = 1; j < sxp2-1; ++j )
				{
					byte* cur = glowBuf + (j + i * sxp2) * 4;
					byte* up = glowBuf + (j + (i+1)*sxp2) * 4;
					byte* down = glowBuf + (j + (i-1)*sxp2) * 4;
					byte* left = glowBuf + (j+1 + i*sxp2) * 4;
					byte* right = glowBuf + (j-1 + i*sxp2) * 4;

					cur[0] = glowColChannel[0];
					cur[1] = glowColChannel[1];
					cur[2] = glowColChannel[2];
					cur[3] = (byte)((float)(up[3] + down[3] + left[3] + right[3]) * 0.25f);
				}
			}
		}

		//now alpha-blend the blur over the original buffer
		for( int i = 0; i < pixelNumber; ++i )
		{
			byte* orig = buf + i*4;
			byte* glow = glowBuf + i*4;

			float s = (float)orig[3] / 255.f; //blend using the alpha in the original buffer

			for( int c = 0; c < 4; ++c )
				orig[c] = (byte)(orig[c] * s + glow[c] * (1.f-s));
		}

		free( glowBuf );
	}

	//drop the buffer in the texture
	loaded = texture->loadFromMemory( buf, sxp2, syp2, GL_RGBA, GL_RGBA );
	texture->disableBilinearFiltering();
	texture->disableMipmaps();
	texture->disableTiling();

	free( buf );

	return loaded;
}


void Font::Page::onUnload(bool soft /*= false */) {
	//a font page can always unload
	texture->onUnload(); //force unload

	loaded = false;
}


/// --------------------------------------------------------------------------------

/// --------------------------------------------------------------------------------

Font::Font( ResourceGroup* creator, const String& path ) :
Resource( creator, path )
{

}

Font::~Font()
{

}

bool Font::onLoad()
{
	DEBUG_ASSERT( !isLoaded(), "onLoad: this font is already loaded" );

	Table t = Platform::singleton().load( filePath );

	fontFile = Utils::getDirectory( filePath ) + '/' + t.getString( "truetype" );
	fontWidth = fontHeight = t.getInt( "size" );	

	antialias = t.getBool( "antialias" );
	kerning = t.getBool( "kerning" );
	generateEdge = t.getBool( "polyOutline" );
	generateSurface = t.getBool( "polySurface" );
	spacing = t.getNumber( "spacing" );

	mPolyOutlineQuality = t.getNumber( "polyOutlineQuality", 10 );

	glowRadius = t.getInt( "glowRadius" );
	glowColor = t.getColor( "glowColor" );

	mCellWidth = fontWidth + glowRadius * 2;
	mCellHeight = fontHeight + glowRadius * 2;

	face = Platform::singleton().getFontSystem().getFace( fontFile );

	auto& preload = t.getTable( "preloadedPages" );
	for( int i = 0; i < preload.getArrayLength(); ++i )
		getPage( preload.getInt( i ) );

	//load existing pages that were trimmed during a previous unload
	for( auto& pair : pages )
	{
		if( !pair.second->isLoaded() )
			pair.second->onLoad();
	}

	return loaded = true;
}

void Font::onUnload( bool soft )
{
	for( auto& pair : pages )
	{
		if( pair.second->isLoaded() )
			pair.second->onUnload();

		if( !soft )
			delete pair.second;
	}

	if( !soft )
		pages.clear();

	loaded = false;
}

float Font::getKerning( Character* next, Character* prev )
{
	DEBUG_ASSERT( kerning, "getKerning: kerning is not enabled on this font" );
	DEBUG_ASSERT( next, "getKerning: next was null" );
	DEBUG_ASSERT( prev, "getKerning: prev was null" );

	FT_Vector vec;

	FT_Get_Kerning( 
		face,
		prev->gliphIdx,
		next->gliphIdx,
		FT_KERNING_DEFAULT,
		&vec );

	return ((float)vec.x * FONT_PPI) / (float)fontWidth;
}

void Font::_prepareFace()
{
	//set dimensions
	FT_Set_Pixel_Sizes(
		face,
		fontWidth,
		fontHeight );
}

int Font::getPixelLength( const String& str )
{
	int l = 0;

	Character* lastChar = nullptr;

	for( size_t i = 0; i < str.size(); ++i )
	{
		Character* chr = getCharacter( str[i] );
		l += (int)(chr->advance * chr->pixelWidth);
		
		if( lastChar && isKerningEnabled() )
			l += (int)(getKerning( chr, lastChar ) * fontWidth);

		lastChar = chr;
	}

	return l;
}

Font::Page* Font::getPage(int index) {
	DEBUG_ASSERT(index < FONT_MAX_PAGES, "getPage: requested page index is past the max page index");

	PageMap::iterator itr = pages.find(index);

	if (itr == pages.end())
	{
		Page* p = new Page(this, index);
		pages[index] = p;
		p->onLoad();
		return p;
	}
	else return itr->second;
}

int Font::getCharIndex(Character* c) {
	return FT_Get_Char_Index(face, c->character);
}

Font::Character* Font::getCharacter(unichar c) {
	return getPageForChar(c)->getChar(c);
}

Texture* Font::getTexture(unichar c) {
	return getPageForChar(c)->getTexture();
}

void Font::preloadPages(const char pages[], int n) {
	for (int i = 0; i < n; ++i)
		getPage(pages[i]);
}
