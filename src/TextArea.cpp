#include "stdafx.h"

#include "TextArea.h"

using namespace Dojo;

TextArea::TextArea( GameState* l, 
		 const std::string& fontSetName, 
		 const Vector& pos, 
		 uint chars, 
		 bool center,
		 const Vector& bounds) :
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
	cullMode = CM_DISABLED;
	inheritAngle = false;

	font = gameState->getFont( fontName );

	DEBUG_ASSERT( font );

	spaceWidth = font->getCharacter(' ')->width;

	//a font always requires alpha
	setRequiresAlpha( true );

	//allocate buffers			
	mesh = new Mesh();
	mesh->setDynamic( true );
	mesh->setTriangleMode( Mesh::TM_LIST );
	mesh->setVertexFieldEnabled( Mesh::VF_POSITION2D );
	mesh->setVertexFieldEnabled( Mesh::VF_UV );
	mesh->setVertexFieldEnabled( Mesh::VF_UV_1 );
	mesh->setVertexFieldEnabled( Mesh::VF_UV_2 );
	mesh->setVertexFieldEnabled( Mesh::VF_UV_3 );
	mesh->setVertexFieldEnabled( Mesh::VF_UV_4 );
	mesh->setVertexFieldEnabled( Mesh::VF_UV_5 );
	mesh->setVertexFieldEnabled( Mesh::VF_UV_6 );
	mesh->setVertexFieldEnabled( Mesh::VF_UV_7 );

	characters = (Font::Character**)malloc( sizeof( void* ) * maxChars );

	absoluteTimeSpeed = true;

	//init
	clearText();
}

TextArea::~TextArea()
{			
	if( mesh )
	{
		mesh->unload();				
		delete mesh;
	}

	free( characters );
}

void TextArea::clearText()
{						
	currentCharIdx = 0;

	//reset characters
	memset( characters, 0, maxChars * sizeof( void* ) );

	content.clear();

	cursorPosition = Vector::ZERO;

	setSize(0,0);

	setVisible( false );

	changed = true;
}

void TextArea::addText( const std::string& text, bool autoLineFeed )
{				
	setVisible( true );

	content += text;

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

void TextArea::addLineFeededText( const std::string& text )
{
	setVisible( true );

	content += text;

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

void TextArea::addText( uint n, char paddingChar, uint digits )
{
	std::string number = Utils::toString( n );

	//if not specified, budget is max char number
	if( digits == 0 )
		digits = maxChars - currentCharIdx;

	//stay in the digit budget?
	if( paddingChar != 0 )
	{				
		//forget most significative digits
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
	}

	addText( number );
}

void TextArea::prepare( const Vector& viewportPixelRatio )
{
	//not changed
	if( !changed )
		return;

	//no characters
	if( !currentCharIdx )
	{
		setVisible( false );
		return;
	}

	//get screen size
	screenSize.x = scale.x = font->getFontWidth() * viewportPixelRatio.x * pixelScale.x;
	screenSize.y = scale.y = font->getFontHeight() * viewportPixelRatio.y * pixelScale.y;

	Font::Character* rep;
	Vector newSize(0,0);
	int set;

	uint lastLineVertexID = 0;

	cursorPosition.x = 0;
	cursorPosition.y = 0;

	//clear textures
	textures.clear();

	//preallocate vertices
	mesh->begin( currentCharIdx * 6 );
	for( uint i = 0; i < currentCharIdx; ++i )
	{
		rep = characters[i];

		set = _findTextureSlot( rep->getTexture() );

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
			mesh->setAllUVs(0,0);
			mesh->uv( rep->uvPos.x, rep->uvPos.y + font->getFontUVSize().y, set );

			mesh->vertex( cursorPosition.x + rep->width, cursorPosition.y );
			mesh->setAllUVs(0,0);
			mesh->uv( rep->uvPos.x + rep->uvWidth, rep->uvPos.y + font->getFontUVSize().y, set);

			mesh->vertex( cursorPosition.x, cursorPosition.y + 1 );
			mesh->setAllUVs(0,0);
			mesh->uv( rep->uvPos.x, rep->uvPos.y, set );

			mesh->vertex( cursorPosition.x + rep->width, cursorPosition.y + 1 );
			mesh->setAllUVs(0,0);
			mesh->uv( rep->uvPos.x + rep->uvWidth, rep->uvPos.y, set );

			mesh->triangle( 0,1,2 );
			mesh->triangle( 1,2,3 );

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