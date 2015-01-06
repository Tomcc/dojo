/*
 *  Font.h
 *  Dojo Training
 *
 *  Created by Tommaso Checchi on 5/27/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "Vector.h"
#include "Color.h"
#include "Resource.h"

#define FONT_PPI (1.f/64.f)

#define FONT_PAGE_SIDE 16
#define FONT_CHARS_PER_PAGE ( FONT_PAGE_SIDE * FONT_PAGE_SIDE )
#define FONT_MAX_PAGES (65535 / FONT_CHARS_PER_PAGE )

namespace Dojo 
{
	class Texture;
	class ResourceGroup;
	class Tessellation;

	class Font : public Resource
	{
	public:

		class Page;
		class Character;
		
		///A Character defines a single Unicode point, its Page, and its texture rect in the Page
		class Character
		{
		public:

			unichar character;
			int gliphIdx;
			
			Vector uvPos;
			float uvWidth, uvHeight;
			
			int pixelWidth;
			float widthRatio, heightRatio; //the ratio of the cell that is occupied
			float advance;

			float bearingU, bearingV;

			Page* page;

			Character();

			void init( Page* p, unichar c, int x, int y, int sx, int sy, FT_Glyph_Metrics* metrics, FT_Outline* outline );

			///return 
			Texture* getTexture();

			///returns the triangle tesselation of this character
			Tessellation* getTesselation();

		protected:

			Unique< Tessellation > mTesselation;
		};
		
		///A Page is a single texture which contains FONT_PAGE_SIDE^2 Characters in Unicode
		class Page : public Resource
		{
		public:
			
			///create a new page for the given index
			Page( Font* font, int index );

			virtual ~Page();

			virtual bool onLoad();

			virtual void onUnload( bool soft = false );

			virtual bool isReloadable()
			{
				return true;
			}

			Texture* getTexture() 
			{
				return texture;
			}

			///get the char in this page
			Character* getChar( unichar c )
			{
				DEBUG_ASSERT( _charInPage(c), "getChar: the requested char doesn't belong to this page" );

				return &( chars[ c - firstCharIdx ] );
			}

			Font* getFont()
			{
				return font;
			}

		protected:

			Font* font;
			Texture* texture;
			int index;
			unichar firstCharIdx;

			Character chars[ FONT_CHARS_PER_PAGE ];

			bool _charInPage( unichar c ) 
			{
				return c >= firstCharIdx && c < (firstCharIdx + FONT_CHARS_PER_PAGE);
			}
		};

		typedef std::unordered_map< int, Page* > PageMap;
		
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

		///returns the maximum width of a character (cell height)
		int getFontWidth()			{	return fontWidth;	}
		///returns the maximum height of a character (cell width)
		int getFontHeight()			{	return fontHeight;	}

		///returns (and lazy-loads) the character Page with the given index
		Page* getPage( int index );

		///returns (and lazy-loads) the Page containing this Unicode character
		Page* getPageForChar( unichar c )
		{
			return getPage( c / FONT_CHARS_PER_PAGE );
		}

		///returns (and lazy-loads) the Character internal representation of this Unicode character
		Character* getCharacter( unichar c );

		///returns (and lazy-loads) the texture which will be bound to render this Unicode character
		Texture* getTexture( unichar c );

		float getKerning( Character* next, Character* prev );

		float getSpacing()
		{
			return spacing;
		}

		///returns the length in pixel for the given string
		int getPixelLength( const String& s );

		int getCharIndex( Character* c );

		///returns how many vertices per unit of lenght are created for the poly outlines of this font
		float getPolyOutlineQuality()
		{
			return mPolyOutlineQuality;
		}

		bool isAntialiased()
		{
			return antialias;
		}

		bool isKerningEnabled()
		{
			return kerning;
		}

		///tells if this Font contains polygon edge informations
		bool hasPolyOutline()
		{
			return generateEdge;
		}

		///tells if this Font contains polygon surface informations
		bool hasPolySurface()
		{
			return generateSurface;
		}

		///forces the loading of the given pages without waiting for lazy-loading
		void preloadPages( const char pages[], int n );
		
	protected:
		
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
		
		static void _blit( byte* dest, FT_Bitmap* bitmap, int x, int y, int destside );
		static void _blitborder( byte* dest, FT_Bitmap* bitmap, int x, int y, int destside, const Color& col );
	};
}

