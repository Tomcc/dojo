/*
 *  TextArea.h
 *  Dojo Training
 *
 *  Created by Tommaso Checchi on 5/24/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "Font.h"
#include "Renderable.h"

namespace Dojo 
{	
	class GameState;

	///TextArea is a Renderable used to display Unicode text
	class TextArea : public Renderable 
	{
	public:
						
		Color fontColor, shadowColor;
		
		Vector pixelScale;
		
		///Creates a new TextArea
		/**\param l the parent object
		\param fontSetName the name of the .font definition file
		\param pos bottom right corner position (unless centered)
		\param center the position is at the center of the string
		\param bounds TextArea's AABB
		*/
		TextArea( Object* l, 
				 const String& fontSetName, 
				 const Vector& pos, 
				 bool center = false,
				 const Vector& bounds = Vector::ONE );
		
		virtual  ~TextArea();
		
		///sets this TextArea to only show up to n of its characters
		/**
		n can be higher than the current character count; all the characters will be shown
		*/
		void setVisibleCharacters( int n )
		{
			if( n != visibleCharsNumber )
			{
				visibleCharsNumber = n;
				
				changed = true;
			}
		}
		
		///increments by 1 the number of visibile characters
		void showNextChar()
		{
			setVisibleCharacters( getVisibleCharacters() + 1 );
		}
				
		///sets the max pixel length for this line.
		void setMaxLineLength( int l );
		
		///sets the space between lines
		void setInterline( float i )		{	interline = i;		}

		///sets an additional spacing between chars (default 0)
		void setCharSpacing( float c )	{	charSpacing = c;	}
		
		///returns the spacing between each line (0-1), proportional to the font height
		float getInterline()				{	return interline;	}

		///returns the height in pixel of a line of this TextArea
		int getLineHeight()
		{
			return font->getFontHeight() * (1 + (int)getInterline() );
		}

		///returns the number of characters that are currently shown
		int getVisibleCharacters()		{	return visibleCharsNumber;	}
		
		///empties this text area
		void clearText();
							
		///adds text to this TextArea
		void addText( const String& text );
		
		///adds a number to this TextArea
		void addText( int n, char paddingChar = 0, int digits = 0 );
					
		///adds a Time string using a number of seconds
		void addTimeSeconds( int n )
		{		
			addText( n / 60 );
			addText( " : " );
			addText( n % 60 );
		}
		
		Font* getFont()					{	return font;			}		

		///returns the number of added chars
		int getLenght()					{	return characters.size();	}

		///returns the text content in String format
		const String& getContent()		{	return content;			}
				
		///returns the size in screen coordinates for UI
		const Vector& getScreenSize()	{	return screenSize;		}

		bool canBeRendered() {
			return actualCharacters > 0;
		}
		
		void _notifyScreenSize( const Vector& ss )
		{
			screenSize.x = ss.x;
			screenSize.y = ss.y;
		}		
		
		virtual void onAction( float dt );
		
	protected:
		
		typedef SmallSet< Renderable* > LayerList;
		typedef SmallSet< Font::Character* > CharacterList;

		String content;
		
		String fontName;
		float spaceWidth, interline, charSpacing;
		int maxLineLenght;
		bool centered;
		
		int currentLineLength, lastSpace;
		
		Font* font;
				
		CharacterList characters;
		bool changed;
		
		float *vertexBuffer, *uvBuffer;		
		size_t visibleCharsNumber;
		
		Vector cursorPosition, screenSize, lastScale;
		Vector mLayersLowerBound, mLayersUpperBound;

		LayerList busyLayers, freeLayers;
		int actualCharacters = 0;
        
		void _prepare();

		void _centerLastLine( int startingAt, float size );

		///create a mesh to be used for text
		Mesh* _createMesh();

		///create a Layer that uses the given Page
		void _pushLayer();

		///get a layer for this page
		Renderable* _enableLayer( Texture& tex );

		///get the layer assigned to this texture
		Renderable* _getLayer( Texture& tex );

		///finishes editing the layers
		void _endLayers();

		///Free any created layer			
		void _hideLayers();

		void _destroyLayer( Renderable& r );

		void _destroyLayers();
	};
}

