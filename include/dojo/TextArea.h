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
		/**\param l the parent level
		\param fontSetName the name of the .font definition file
		\param pos bottom right corner position (unless centered)
		\param center the position is at the center of the string
		\param bounds TextArea's AABB
		*/
		TextArea( GameState* l, 
				 const String& fontSetName, 
				 const Vector& pos, 
				 bool center = false,
				 const Vector& bounds = Vector::ONE );
		
		virtual  ~TextArea();
		
		///sets this TextArea to only show up to n of its characters
		/**
		n can be higher than the current character count; all the characters will be shown
		*/
		inline void setVisibleCharacters( uint n )
		{
			if( n != visibleCharsNumber )
			{
				visibleCharsNumber = n;
				
				changed = true;
			}
		}
		
		///increments by 1 the number of visibile characters
		inline void showNextChar()
		{
			setVisibleCharacters( getVisibleCharacters() + 1 );
		}
				
		///sets the max pixel length for this line.
		void setMaxLineLength( uint l );
		
		///sets the space between lines
		inline void setInterline( float i )		{	interline = i;		}

		///sets an additional spacing between chars (default 0)
		inline void setCharSpacing( float c )	{	charSpacing = c;	}
		
		inline float getInterline()				{	return interline;	}

		///returns the number of characters that are currently shown
		inline uint getVisibleCharacters()		{	return visibleCharsNumber;	}
		
		///empties this text area
		void clearText();
							
		///adds text to this TextArea
		void addText( const String& text );
		
		///adds a number to this TextArea
		void addText( uint n, char paddingChar = 0, uint digits = 0 );
					
		///adds a Time string using a number of seconds
		inline void addTimeSeconds( uint n )
		{		
			addText( n / 60 );
			addText( " : " );
			addText( n % 60 );
		}
		
		inline Font* getFont()					{	return font;			}		

		///returns the number of added chars
		inline uint getLenght()					{	return characters.size();	}

		///returns the text content in String format
		inline const String& getContent()		{	return content;			}
				
		///returns the size in screen coordinates for UI
		inline const Vector& getScreenSize()	{	return screenSize;		}
		
		inline void _notifyScreenSize( const Vector& ss )
		{
			screenSize.x = ss.x;
			screenSize.y = ss.y;
		}		
		
		virtual void onAction( float dt );
		
	protected:
		
		typedef Array< Renderable* > LayerList;
		typedef Array< Font::Character* > CharacterList;

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
		int visibleCharsNumber;
		
		Vector cursorPosition, screenSize, lastScale;
		Vector mLayersLowerBound, mLayersUpperBound;

		LayerList busyLayers, freeLayers;
        
		void _prepare();

		void _centerLastLine( uint startingAt, float size );

		///create a mesh to be used for text
		Mesh* _createMesh();

		///create a Layer that uses the given Page
		Renderable* _createLayer( Texture* t );

		///get a layer for this page
		Renderable* _enableLayer( Texture* tex );

		///get the layer assigned to this texture
		inline Renderable* _getLayer( Texture* tex )
		{
			DEBUG_ASSERT( tex, "Cannot get a layer for a null texture" );

			//find this layer in the already assigned, or get new
			for( int i = 0; i < busyLayers.size(); ++i )
			{
				if( busyLayers[i]->getTexture() == tex )
					return busyLayers[i];
			}

			//does not exist, hit a free one
			return _enableLayer( tex );
		}

		///finishes editing the layers
		void _endLayers();

		///Free any created layer			
		void _hideLayers();

		void _destroyLayer( Renderable* r );

		inline void _destroyLayers()
		{
			for( int i = 0; i < busyLayers.size(); ++i )
				_destroyLayer( busyLayers[i] );

			for( int i = 0; i < freeLayers.size(); ++i )
				_destroyLayer( freeLayers[i] );
		}
	};
}

#endif