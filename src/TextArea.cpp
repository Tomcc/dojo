#include "stdafx.h"

#include "TextArea.h"

using namespace Dojo;

void TextArea::prepare( const Vector& viewportPixelRatio )
{
	if( !changed || !currentCharIdx )
		return;

	//get screen size
	screenSize.x = scale.x = font->getFontWidth() * viewportPixelRatio.x * pixelScale.x;
	screenSize.y = scale.y = font->getFontHeight() * viewportPixelRatio.y * pixelScale.y;

	Font::Character* rep;
	Vector newSize(0,0);

	uint lastLineVertexID = 0;

	cursorPosition.x = 0;
	cursorPosition.y = 0;

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
			cursorPosition.x += spaceWidth*4;

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
	newSize.y = -cursorPosition.y+1 * screenSize.y * 1.5f;
	setSize(newSize);	

	//push the mesh on the GPU
	mesh->end();

	changed = false;
}