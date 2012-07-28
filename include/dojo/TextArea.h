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
				
		///sets the max pixel lenght for this line.
		void setMaxLineLength( uint l );
		
		inline void setInterline( float i )		{	interline = i;		}
		inline void setCharSpacing( float c )	{	charSpacing = c;	}
		
		inline float getInterline()				{	return interline;	}
		inline uint getVisibleCharacters()		{	return visibleCharsNumber;	}
		
		void clearText();
							
		///async text update
		void addText( const String& text );
		
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
		
		virtual void onAction( float dt );
		
	protected:
		
		typedef Array< Renderable* > LayerList;

		String content;
		
		String fontName;
		float spaceWidth, interline, charSpacing;
		int maxChars;
		int maxLineLenght;
		bool centered;
		
		int currentLineLength, lastSpace;
		
		Font* font;
				
		Font::Character** characters;
		bool changed;
		
		float *vertexBuffer, *uvBuffer;		
		int currentCharIdx, visibleCharsNumber;
		
		Vector cursorPosition, screenSize, lastScale;
		Vector mLayersLowerBound, mLayersUpperBound;

		LayerList busyLayers, freeLayers;

		void _centerLastLine( uint startingAt, float size );

		///create a mesh to be used for text
		Mesh* _createMesh();

		///create a Layer that uses the given Page
		inline Renderable* _createLayer( Texture* t )
		{
			DEBUG_ASSERT( t );

			Renderable* r = new Renderable( gameState, Vector::ZERO );
			r->scale = scale;
			r->setMesh( _createMesh() );
			r->setTexture( t );
			r->setVisible( false );
			r->setActive( false );

			addChild( r, getLayer() );
			freeLayers.add( r );

			return r;
		}

		///get a layer for this page
		Renderable* _enableLayer( Texture* tex );

		///get the layer assigned to this texture
		inline Renderable* _getLayer( Texture* tex )
		{
			DEBUG_ASSERT( tex );

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