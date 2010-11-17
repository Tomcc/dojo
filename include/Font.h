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

#include "dojo_config.h"

#include <iostream>

#include "Vector.h"
#include "Texture.h"
#include "FrameSet.h"

#include "Utils.h"

namespace Dojo {
	class Font
	{
	public:
		
		struct Character
		{
		public:			
			unichar character;
			
			Vector uvPos;
			float uvWidth;
			
			uint pixelWidth;
			float width; //charWidth/fontWidth
			
			Texture* page;
			
			Character() :
			page( NULL ),
			character(0)
			{
				
			}			
		};
		
		struct Page
		{
		public:
			
			Character* chars;
			
			Texture* texture;
			
			Page( Texture* page ) :
			texture( page )
			{
				chars = new Character[ 256 ];	
				
				for( uint i = 0; i < 256; ++i )
					chars[i].page = texture;
			}
		};
		
		Font( const std::string& path, FrameSet* set ) :
		fontSet( set ),
		filePath( path )
		{			
			//allocate page pointers (but not pages)
			pages = (Page**)malloc( sizeof( void* ) * fontSet->getFrameNumber() );
			
			//allocate char widths
			charWidth = (uint*)malloc( sizeof( uint ) * fontSet->getFrameNumber() * 256 );
		}
		
		~Font()
		{
			unload();
		}
		
		inline uint getFontWidth()					{	return fontWidth;		}
		inline uint getFontHeight()					{	return fontHeight;		}
		
		inline const Vector& getFontUVSize()		{	return fontUVSize;		}
		inline FrameSet* getFrameSet()				{	return fontSet;			}
				
		///returns the texture page and the uv min and max for that character
		inline Character* getCharacter( unichar c )
		{
			//TODO: support multiple pages
			c = c % 256;
			Character* res = pages[ 0 ]->chars + (c % 256);
			return res;
		}
		
		bool load()
		{
			NSString* path = Utils::toNSString( filePath );
			//load spacings from file
			NSData* data = [NSData dataWithContentsOfFile:path];
			
			DEBUG_MESSAGE( "loading contents of file" << filePath );
			 
			if( data == nil )	return false;
			 
			char* buf = (char*)data.bytes;
			char* end = buf + [data length];
			
			//the first two values are pixel width and height of the font
			fontWidth = Utils::toInt( buf, end );
			fontHeight = Utils::toInt( buf, end );
			
			offset = Utils::toInt( buf, end );
			
			//get UV dimensions
			Texture* tex = fontSet->getFrame(0);
			
			cols = (uint)tex->getWidth() / fontWidth;
			rows = (uint)tex->getHeight() / fontHeight;
			
			totalCharNumber = rows*cols;
						
			//find height of a char in UV space
			fontUVSize.x = (float)fontWidth/(float)( tex->getWidth() );
			fontUVSize.y = (float)fontHeight/(float)( tex->getHeight() );			
			
			//get ints representing char width in pixels
			uint i = 0;
			for (; i < 256 && buf < end; ++i)
			{
				charWidth[i] = Utils::toInt( buf, end );
				
				DEBUG_OUT( charWidth[i] );
			}
				
			DOJO_ASSERT( i == 256 );
			
			_createPage( 0 );			
						
			return true;
		}
		
		void unload()
		{
			if( fontSet && pages && charWidth )
			{
				free( pages );
				free( charWidth );
				
				pages = NULL;
				charWidth = NULL;
				
				fontSet = NULL;
			}
		}
		
	protected:
		
		std::string filePath;
		
		FrameSet* fontSet;
		
		Page** pages; 
		
		uint* charWidth; //width for each char
		
		uint offset;
		uint rows, cols, totalCharNumber;
		uint fontWidth, fontHeight; //measurements of the "character box"
		
		Vector fontUVSize;	//char max size in UV metrics
		
		//creates the given page
		bool _createPage( uint n )
		{
			if( n >= fontSet->getFrameNumber() )
				return false;
			
			pages[ n ] = new Page( fontSet->getFrame( n ) );
			
			Character* chars = pages[n]->chars;
			
			//get the widths for this page
			uint* pageCharWidth = charWidth + n * 256;
			
			//load characters settings
			Vector uv;
								
			uint i = offset;
			uint j = 0;
			Character* c;
			for( ; i < totalCharNumber; ++i, ++j )
			{
				//next line?
				if( j == cols )
				{				
					uv.y += fontUVSize.y;
					uv.x = 0;
					
					j = 0;					
				}
				
				c = chars + i;
				
				c->character = i;
				c->pixelWidth = pageCharWidth[i];
				c->width = (float)c->pixelWidth/(float)fontWidth;
				
				c->uvPos = uv;				
				c->uvWidth = c->width * fontUVSize.x;
				
				//next char
				uv.x += fontUVSize.x;				
			}
			
			return true;
		}
	};
}

#endif