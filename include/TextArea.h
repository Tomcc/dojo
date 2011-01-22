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

#include <string>

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
				 const std::string& fontSetName, 
				 const Vector& pos, 
				 uint chars, 
				 bool center = false,
				 const Vector& bounds = Vector::ONE ) :
		Renderable( l, pos ),
		fontName( fontSetName ),
		maxChars( chars ),
		interline( 0.2f ),
		charSpacing( 0 ),
		maxLineLenght( 0xfffffff ),
		centered( center ),
		pixelScale( 1,1 )
		{		
			setSize( bounds );
						
			font = gameState->getFont( fontName );
			
			DEBUG_ASSERT( font );
			DEBUG_ASSERT( font->getFrameSet() );
			DEBUG_ASSERT( font->getFrameSet()->getFrameNumber() );
			
			texture = font->getFrameSet()->getFrame(0);
			
			spaceWidth = font->getCharacter(' ')->width;
			
			//a font always requires alpha
			setRequiresAlpha( true );
			
			//allocate buffers			
			mesh = new Mesh();
			mesh->setDynamic( true );
			mesh->setTriangleMode( Mesh::TM_LIST );
			mesh->setVertexFieldEnabled( Mesh::VF_POSITION2D, true);
			mesh->setVertexFieldEnabled( Mesh::VF_UV, true);
						
			characters = (Font::Character**)malloc( sizeof( void* ) * maxChars );
						
			absoluteTimeSpeed = true;
			
			//init
			clearText();
		}
		
		virtual  ~TextArea()
		{			
			if( mesh )
			{
				mesh->unload();				
				delete mesh;
			}
		}
				
		inline void setMaxLineLenght( uint l )	{	maxLineLenght = l;	}
		inline void setInterline( float i )		{	interline = i;		}
		inline void setCharSpacing( float c )	{	charSpacing = c;	}
		
		inline float getInterline()				{	return interline;	}
		
		inline void clearText()
		{						
			currentCharIdx = 0;
			
			//reset characters
			memset( characters, 0, maxChars * sizeof( void* ) );
						
			cursorPosition = Vector::ZERO;
			
			setSize(0,0);
			
			changed = true;
		}
							
		///async text update
		void addText( const std::string& text, bool autoLineFeed = false )		
		{							
			//parse and setup characters
			for( uint i = 0; i < text.size() && currentCharIdx < maxChars; ++i, ++currentCharIdx )
			{
				if( text.at( i ) == '\n' )
					characters[currentCharIdx] = NULL;
					
				else
					characters[currentCharIdx] = font->getCharacter( text.at(i) );
			}
			
			changed = true;
		}
		
		void addLineFeededText( const std::string& text )
		{
			uint currentLineLenght = 0;
			
			uint lastSpace = 0;
			Font::Character* currentChar;
			char c;
			
			//parse and setup characters
			for( uint i = 0; i < text.size() && currentCharIdx < maxChars; ++i, ++currentCharIdx )
			{
				c = text.at(i);
				
				if( c < 0 )
					continue;
				
				currentChar = characters[currentCharIdx] = font->getCharacter( c );
				
				currentLineLenght += currentChar->pixelWidth;

				if( c == ' ' || c == '\t' )
					lastSpace = currentCharIdx;
				
				else if( c == '\n' )
				{
					lastSpace = 0;
					characters[currentCharIdx] = NULL;
					currentLineLenght = 0;
				}
				
				//lenght eccess? find last whitespace and replace with \n.
				if( currentLineLenght > maxLineLenght && lastSpace )
				{
					characters[lastSpace] = NULL;
					lastSpace = 0;
					currentLineLenght = 0;
				}
			}
			
			changed = true;			
		}
		
		inline void addText( uint n, char paddingChar = 0, uint digits = 0 )
		{
			std::string number = Utils::toString( n );
			
			//if not specified, budget is max char number
			if( digits == 0 )
				digits = maxChars - currentCharIdx;
			
			//stay in the digit budget?
			if( paddingChar != 0 )
			{				
				//forget most significative numbers
				if( number.size() > digits )
					number = number.substr( number.size() - digits );
				
				//pad to fill
				else if( number.size() < digits )
				{
					std::string padding;
					
					for( uint i = 0; i < digits - number.size(); ++i )
						padding += paddingChar;
					
					number = padding + number;
				}				
				
				//HACK
				if( n == 0 )
				{
					number.clear();
					for( uint i = 0; i < digits; ++i )
						number += "0";
				}
			}
			
			addText( number );
		}
		
		inline void addTimeSeconds( uint n )
		{		
			addText( n / 60 );
			addText( " : " );
			addText( n % 60 );
		}
		
		inline Font* getFont()					{	return font;			}		
		inline uint getLenght()					{	return currentCharIdx;	}
				
		inline const Vector& getScreenSize()		{	return screenSize;		}	
		
		void prepare( const Vector& viewportPixelRatio )
		{
			if( !changed )
				return;
			
			//get screen size
			screenSize.x = scale.x = font->getFontWidth() * viewportPixelRatio.x * pixelScale.x;
			screenSize.y = scale.y = font->getFontHeight() * viewportPixelRatio.y * pixelScale.x;
			
			Font::Character* rep;
			Vector newSize(0,0);
			
			uint lastLineVertexID = 0;
			
			//preallocate vertices
			mesh->begin( currentCharIdx * 6 );
			for( uint i = 0; i < currentCharIdx; ++i )
			{
				rep = characters[i];
								
				//avoid special chars
				if( rep == NULL )
				{									
					//if centered move every character of this line along x of 1/2 size
					if( centered )
						_centerLastLine( lastLineVertexID, cursorPosition.x );
					
					cursorPosition.y -= 1.f + interline;
					cursorPosition.x = 0;
					
					lastLineVertexID = mesh->getVertexCount();
				}
				else if( rep->character == '\t' )
					cursorPosition.y += spaceWidth*4;
				
				else if( rep->character == ' ' )
					cursorPosition.x += spaceWidth;
				
				else	//real character
				{										
					//assign vertex positions					
					//and uv coordinates
					mesh->vertex( cursorPosition.x, cursorPosition.y );
					mesh->uv( rep->uvPos.x, rep->uvPos.y + font->getFontUVSize().y );
					
					mesh->vertex( cursorPosition.x + rep->width, cursorPosition.y );
					mesh->uv( rep->uvPos.x + rep->uvWidth, rep->uvPos.y + font->getFontUVSize().y);
					
					mesh->vertex( cursorPosition.x, cursorPosition.y + 1 );
					mesh->uv( rep->uvPos.x, rep->uvPos.y );
					
					mesh->vertex( cursorPosition.x + rep->width, cursorPosition.y );
					mesh->uv( rep->uvPos.x + rep->uvWidth, rep->uvPos.y + font->getFontUVSize().y );
					
					mesh->vertex( cursorPosition.x, cursorPosition.y + 1 );
					mesh->uv( rep->uvPos.x, rep->uvPos.y );
										
					mesh->vertex( cursorPosition.x + rep->width, cursorPosition.y + 1 );
					mesh->uv( rep->uvPos.x + rep->uvWidth, rep->uvPos.y );
										
					//now move to the next character
					cursorPosition.x += rep->width + charSpacing;
				}	
				
				//update size to contain cursorPos to the longest line
				if( cursorPosition.x > newSize.x )
					newSize.x = cursorPosition.x;		
			}
					
			
			//if centered move every character of this line along x of 1/2 size
			if( centered )
				_centerLastLine( lastLineVertexID, cursorPosition.x );
						
			//set new size
			newSize.x *= screenSize.x;
			newSize.y = -cursorPosition.y * screenSize.y * 1.5f;
			setSize(newSize);	
			
			//push the mesh on the GPU
			mesh->end();
		
			changed = false;
		}
		
		inline void _notifyScreenSize( const Vector& ss )
		{
			screenSize.x = ss.x;
			screenSize.y = ss.y;
		}		
		
	protected:
		
		inline void _centerLastLine( uint startingAt, float size )
		{
			if( mesh->getVertexCount() == 0 )
				return;
			
			float halfWidth = size * 0.5f;
						
			for( uint i = startingAt; i < mesh->getVertexCount(); ++i )
				*(mesh->_getVertex( i )) -= halfWidth;		
		}
		
		std::string fontName;
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
	};
}

#endif