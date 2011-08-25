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

#include "Sprite.h"
#include "Font.h"
#include "GameState.h"

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
		
	protected:
		
		typedef Array< Renderable* > LayerList;

		String content;
		
		String fontName;
		float spaceWidth, interline, charSpacing;
		uint maxChars;
		uint maxLineLenght;
		bool centered;
		
		uint currentLineLength, lastSpace;
		
		Font* font;
				
		Font::Character** characters;
		bool changed;
		
		float *vertexBuffer, *uvBuffer;		
		uint currentCharIdx, visibleCharsNumber;
		
		Vector cursorPosition, screenSize, lastScale;

		LayerList busyLayers, freeLayers;

		inline void _centerLastLine( uint startingAt, float size )
		{
			if( mesh->getVertexCount() == 0 )
				return;

			float halfWidth = size * 0.5f;

			for( uint i = startingAt; i < mesh->getVertexCount(); ++i )
				*(mesh->_getVertex( i )) -= halfWidth;		
		}

		///create a mesh to be used for text
		inline Mesh* _createMesh()
		{
			Mesh * mesh = new Mesh();
			mesh->setDynamic( true );
			mesh->setTriangleMode( Mesh::TM_LIST );
			mesh->setVertexFieldEnabled( Mesh::VF_POSITION2D );
			mesh->setVertexFieldEnabled( Mesh::VF_UV );

			return mesh;
		}

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
		inline Renderable* _enableLayer( Texture* tex )
		{
			if( freeLayers.size() == 0 )
				_createLayer( tex );

			Renderable* r = freeLayers.top();
			freeLayers.pop();

			r->setVisible( true );
			r->setActive( true );
			r->setTexture( tex );

			r->getMesh()->begin( currentCharIdx * 2 );

			busyLayers.add( r );

			return r;
		}

		///get the layer assigned to this texture
		inline Renderable* _getLayer( Texture* tex )
		{
			DEBUG_ASSERT( tex );

			//find this layer in the already assigned, or get new
			for( uint i = 0; i < busyLayers.size(); ++i )
			{
				if( busyLayers[i]->getTexture() == tex )
					return busyLayers[i];
			}

			//does not exist, hit a free one
			return _enableLayer( tex );
		}

		///finishes editing the layers
		inline void _endLayers()
		{
			for( uint i = 0; i < busyLayers.size(); ++i )
				busyLayers[i]->getMesh()->end();

			//also end this
			if( mesh->isEditing() )
				mesh->end();
		}

		///Free any created layer			
		inline void _hideLayers()
		{
			for( uint i = 0; i < busyLayers.size(); ++i )
			{
				Renderable* l = busyLayers[i];

				l->setVisible( false );
				l->setActive( false );

				freeLayers.add( l );
			}

			busyLayers.clear();
		}

		inline void _destroyLayer( Renderable* r )
		{
			DEBUG_ASSERT( r );

			if( r == this )  //do not delete the TA itself, even if it is a layer
				return;

			delete r->getMesh();

			removeChild( r );
			busyLayers.remove( r );
			freeLayers.remove( r );

			delete r;
		}

		inline void _destroyLayers()
		{
			for( uint i = 0; i < busyLayers.size(); ++i )
				_destroyLayer( busyLayers[i] );

			for( uint i = 0; i < freeLayers.size(); ++i )
				_destroyLayer( freeLayers[i] );
		}
	};
}

#endif