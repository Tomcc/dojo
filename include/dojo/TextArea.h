/*
 *  TextArea.h
 *  Dojo Training
 *
 *  Created by Tommaso Checchi on 5/24/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef TextArea_h__
#define TextArea_h__

#include "dojo/dojo_common_header.h"

#include "dojo/Sprite.h"
#include "dojo/Font.h"
#include "dojo/GameState.h"

namespace Dojo 
{	
	class TextArea : public Renderable 
	{
	public:
						
		Color fontColor, shadowColor;
		
		Vector pixelScale;
		
		TextArea( GameState* l, 
				 const String& fontSetName, 
				 const Vector& pos, 
				 uint chars, 
				 bool center = false,
				 const Vector& bounds = Vector::ONE );
		
		virtual  ~TextArea();
				
		inline void setMaxLineLenght( uint l )	{	maxLineLenght = l;	}
		inline void setInterline( float i )		{	interline = i;		}
		inline void setCharSpacing( float c )	{	charSpacing = c;	}
		
		inline float getInterline()				{	return interline;	}
		
		void clearText();
							
		///async text update
		void addText( const String& text );
		
		void addLineFeededText( const String& text );
		
		void addText( uint n, char paddingChar = 0, uint digits = 0 );
					
		inline void addTimeSeconds( uint n )
		{		
			addText( n / 60 );
			addText( " : " );
			addText( n % 60 );
		}
		
		inline Font* getFont()					{	return font;			}		
		inline uint getLenght()					{	return currentCharIdx;	}
		inline const String& getContent()	{	return content;			}
				
		inline const Vector& getScreenSize()	{	return screenSize;		}	
		
		bool prepare( const Vector& viewportPixelRatio );
		
		inline void _notifyScreenSize( const Vector& ss )
		{
			screenSize.x = ss.x;
			screenSize.y = ss.y;
		}		
		
	protected:
		
		String content;
		
		String fontName;
		float spaceWidth, interline, charSpacing;
		uint maxChars;
		uint maxLineLenght;
		bool centered;
		
		Font* font;
				
		Font::Character** characters;
		bool changed;
		
		float *vertexBuffer, *uvBuffer;		
		uint currentCharIdx;
		
		Vector cursorPosition, screenSize;	

		inline void _centerLastLine( uint startingAt, float size )
		{
			if( mesh->getVertexCount() == 0 )
				return;

			float halfWidth = size * 0.5f;

			for( uint i = startingAt; i < mesh->getVertexCount(); ++i )
				*(mesh->_getVertex( i )) -= halfWidth;		
		}

		inline uint _findTextureSlot( Texture* tex ) 
		{
			DEBUG_ASSERT( tex );

			int set = textures.getElementIndex( tex );

			if( set == -1 )
			{
				DEBUG_ASSERT( textures.size() < MESH_MAX_TEXTURES );

				set = textures.size();
				textures.add( tex );
			}

			return set;
		}
	};
}

#endif