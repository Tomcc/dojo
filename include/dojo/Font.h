/*
 *  Font.h
 *  Dojo Training
 *
 *  Created by Tommaso Checchi on 5/27/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef Font_h__
#define Font_h__

#include "dojo_common_header.h"

#include "Vector.h"
#include "Texture.h"
#include "FrameSet.h"

#include "Tessellation.h"

#include "Utils.h"

#define FONT_PPI (1.f/64.f)

#define FONT_PAGE_SIDE 16
#define FONT_CHARS_PER_PAGE ( FONT_PAGE_SIDE * FONT_PAGE_SIDE )
#define FONT_MAX_PAGES (65535 / FONT_CHARS_PER_PAGE )

namespace Dojo 
{
	class ResourceGroup;

	class Font : public Dojo::Resource
	{
	public:

		class Page;
		class Character;
		
		///A Character defines a single Unicode point, its Page, and its texture rect in the Page
		class Character
		{
		public:

			unichar character;
			uint gliphIdx;
			
			Vector uvPos;
			float uvWidth, uvHeight;
			
			uint pixelWidth;
			float widthRatio, heightRatio; //the ratio of the cell that is occupied
			float advance;

			float bearingU, bearingV;

			Page* page;

			Character()
			{

			}

			void init( Page* p, unichar c, int x, int y, int sx, int sy, FT_Glyph_Metrics* metrics, FT_Outline* outline );

			///return 
			inline Texture* getTexture()
			{
				return page->getTexture();
			}

			///returns the triangle tesselation of this character
			Tessellation* getTesselation()
			{
				DEBUG_ASSERT( mTesselation.get(), "Tried to get a tesselation but this Font wasn't tesselated (forgot to specify 'tesselate' in the config?)" );

				return mTesselation.get();
			}

		protected:

			std::unique_ptr< Tessellation > mTesselation;
		};
		
		///A Page is a single texture which contains FONT_PAGE_SIDE^2 Characters in Unicode
		class Page : public Resource
		{
		public:
			
			///create a new page for the given index
			Page( Font* font, int index );

			virtual ~Page()
			{
				SAFE_DELETE( texture );
			}

			virtual bool onLoad();

			virtual void onUnload( bool soft = false )
			{
				//a font page can always unload
				texture->onUnload(); //force unload

				loaded = false;
			}

			virtual bool isReloadable()
			{
				return true;
			}

			inline Texture* getTexture() 
			{
				return texture;
			}

			///get the char in this page
			inline Character* getChar( unichar c )
			{
				DEBUG_ASSERT( _charInPage(c), "getChar: the requested char doesn't belong to this page" );

				return &( chars[ c - firstCharIdx ] );
			}

			inline Font* getFont()
			{
				return font;
			}

		protected:

			Font* font;
			Texture* texture;
			uint index;
			unichar firstCharIdx;

			Character chars[ FONT_CHARS_PER_PAGE ];

			inline bool _charInPage( unichar c ) 
			{
				return c >= firstCharIdx && c < (firstCharIdx + FONT_CHARS_PER_PAGE);
			}
		};

		typedef std::unordered_map< uint, Page* > PageMap;
		
		///A Font represents a single .font file, and is bound to a .ttf TrueType font definition
		/**
		\param creator the ResourceGroup which created this Resource
		\path the path to the .font definition file
		*/
		Font( ResourceGroup* creator, const String& path );
		
		virtual ~Font();

		virtual bool onLoad();
		///purges all the loaded pages from memory and prompts a rebuild
		virtual void onUnload( bool soft = false );

		inline const String& getName()
		{
			return fontName;
		}
		
		///returns the maximum width of a character (cell height)
		inline uint getFontWidth()			{	return fontWidth;	}
		///returns the maximum height of a character (cell width)
		inline uint getFontHeight()			{	return fontHeight;	}

		///returns (and lazy-loads) the character Page with the given index
		inline Page* getPage( uint index )
		{
			DEBUG_ASSERT( index < FONT_MAX_PAGES, "getPage: requested page index is past the max page index" );

			PageMap::iterator itr = pages.find( index );

			if( itr == pages.end() )
			{
				Page* p = new Page( this, index );
				pages[ index ] = p;
				p->onLoad();
				return p;
			}
			else return itr->second;
		}

		///returns (and lazy-loads) the Page containing this Unicode character
		inline Page* getPageForChar( unichar c )
		{
			return getPage( c / FONT_CHARS_PER_PAGE );
		}

		///returns (and lazy-loads) the Character internal representation of this Unicode character
		inline Character* getCharacter( unichar c )
		{
			return getPageForChar( c )->getChar( c );
		}

		///returns (and lazy-loads) the texture which will be bound to render this Unicode character
		inline Texture* getTexture( unichar c )
		{
			return getPageForChar( c )->getTexture();
		}

		float getKerning( Character* next, Character* prev );

		inline float getSpacing()
		{
			return spacing;
		}

		///returns the length in pixel for the given string
		int getPixelLength( const String& s );

		inline uint getCharIndex( Character* c )
		{
			return FT_Get_Char_Index( face, c->character );
		}

		///returns how many vertices per unit of lenght are created for the poly outlines of this font
		float getPolyOutlineQuality()
		{
			return mPolyOutlineQuality;
		}

		inline bool isAntialiased()
		{
			return antialias;
		}

		inline bool isKerningEnabled()
		{
			return kerning;
		}

		///tells if this Font contains polygon edge informations
		inline bool hasPolyOutline()
		{
			return generateEdge;
		}

		///tells if this Font contains polygon surface informations
		inline bool hasPolySurface()
		{
			return generateSurface;
		}

		///forces the loading of the given pages without waiting for lazy-loading
		inline void preloadPages( const char pages[], uint n ) 
		{
			for( uint i = 0; i < n; ++i )
				getPage( pages[i] );
		}
		
	protected:
		
		String fontName;
		String fontFile;

		bool antialias, kerning, generateEdge, generateSurface;
		float mPolyOutlineQuality;

		float spacing;

		int fontWidth, fontHeight; //clean measurements for the typeface
		int mCellWidth, mCellHeight; //measurements of the "character box" 
		
		int glowRadius;
		Color glowColor;

		FT_Face face;

		///this has to be called each time that we need to use the face
		void _prepareFace();
		
		PageMap pages;
		
		static void _blit( byte* dest, FT_Bitmap* bitmap, uint x, uint y, uint destside );
		static void _blitborder( byte* dest, FT_Bitmap* bitmap, uint x, uint y, uint destside, const Color& col );
	};
}

#endif