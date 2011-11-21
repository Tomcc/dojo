#include "stdafx.h"

#include "TextArea.h"
#include "Game.h"
#include "GameState.h"
#include "Mesh.h"

using namespace Dojo;

TextArea::TextArea( GameState* l, 
		 const String& fontSetName, 
		 const Vector& pos, 
		 uint chars, 
		 bool center,
		 const Vector& bounds) :
Renderable( l, pos ),
fontName( fontSetName ),
maxChars( chars ),
interline( 0.2f ),
maxLineLenght( 0xfffffff ),
centered( center ),
pixelScale( 1,1 ),
currentLineLength( 0 ),
lastSpace( 0 ),
visibleCharsNumber( 0xffffffff )
{		
	setSize( bounds );
	cullMode = CM_DISABLED;
	inheritAngle = false;

	font = gameState->getFont( fontName );

	DEBUG_ASSERT( font );

	charSpacing = font->getSpacing();
	spaceWidth = font->getCharacter(' ')->advance;

	//a font always requires alpha
	setRequiresAlpha( true );

	//allocate buffers			
	mesh = _createMesh();
	freeLayers.add( this ); //add itself to the free layers

	characters = (Font::Character**)malloc( sizeof( void* ) * maxChars );

	//init
	clearText();
}

TextArea::~TextArea()
{			
	if( mesh )
	{
		mesh->unload();				
		SAFE_DELETE( mesh );
	}

	_destroyLayers();

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

	changed = true;
	
	visibleCharsNumber = 0xffffffff;
	currentLineLength = 0;
	lastSpace = 0;
}

void TextArea::setMaxLineLength( uint l )
{		
	//HACK PAZZESCOH
	maxLineLenght = l * ((float)gameState->getGame()->getNativeWidth() / (float)640 );
}

void TextArea::addText( const String& text )
{
	content += text;

	Font::Character* currentChar;
	unichar c;

	//parse and setup characters
	for( uint i = 0; i < text.size() && currentCharIdx < maxChars; ++i, ++currentCharIdx )
	{
		c = text[i];

		currentChar = characters[currentCharIdx] = font->getCharacter( c );

		currentLineLength += currentChar->pixelWidth;

		if( c == ' ' || c == '\t' )
			lastSpace = currentCharIdx;

		else if( c == '\n' )
		{
			lastSpace = 0;
			currentLineLength = 0;
		}

		//lenght eccess? find last whitespace and replace with \n.
		if( currentLineLength > maxLineLenght && lastSpace )
		{
			characters[lastSpace] = font->getCharacter('\n');
			lastSpace = 0;
			currentLineLength = 0;
		}
	}

	changed = true;			
}

void TextArea::addText( uint n, char paddingChar, uint digits )
{
	String number( n );

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
			String padding;

			for( uint i = 0; i < digits - number.size(); ++i )
				padding += paddingChar;

			number = padding + number;
		}
	}

	addText( number );
}

Renderable* TextArea::_enableLayer( Texture* tex )
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

void TextArea::_endLayers()
{
	for( uint i = 0; i < busyLayers.size(); ++i )
		busyLayers[i]->getMesh()->end();
	
	//also end this
	if( mesh->isEditing() )
		mesh->end();
}

///Free any created layer			
void TextArea::_hideLayers()
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

void TextArea::_destroyLayer( Renderable* r )
{
	DEBUG_ASSERT( r );
	
	if( r == this )  //do not delete the TA itself, even if it is a layer
		return;
	
	delete r->getMesh();
	
	removeChild( r );
	busyLayers.remove( r );
	freeLayers.remove( r );
	
	SAFE_DELETE( r );
}


bool TextArea::prepare( const Vector& viewportPixelRatio )
{
	//not changed
	if( !changed )
		return true;

	//no characters to show
	if( !visibleCharsNumber || !currentCharIdx )
		return false;

	//get screen size
	screenSize.x = scale.x = font->getFontWidth() * viewportPixelRatio.x * pixelScale.x;
	screenSize.y = scale.y = font->getFontHeight() * viewportPixelRatio.y * pixelScale.y;

	Font::Character* rep, *lastRep = NULL;
	Vector newSize(0,0);
	bool doKerning = font->isKerningEnabled();
	int lastLineVertexID = 0;
	int idx = 0;

	cursorPosition.x = 0;
	cursorPosition.y = 0;

	//clear layers
	_hideLayers();

	//either reach the last valid character or the last existing character
	for( int i = 0; i < visibleCharsNumber && i < currentCharIdx; ++i )
	{
		rep = characters[i];

		//avoid to rendering spaces
		if( rep->character == '\n' )
		{									
			//if centered move every character of this line along x of 1/2 size
			if( centered ) 
			{
				_centerLastLine( lastLineVertexID, cursorPosition.x );
				lastLineVertexID = mesh->getVertexCount();
			}

			cursorPosition.y -= 1.f + interline;
			cursorPosition.x = 0;
			lastRep = NULL;
		}
		else if( rep->character == '\t' )
		{
			cursorPosition.x += spaceWidth*4;
			lastRep = NULL;
		}
		else if( rep->character == ' ' )
		{
			cursorPosition.x += spaceWidth;
			lastRep = NULL;
		}
		else	//real character
		{
			Mesh* layer = _getLayer( rep->getTexture() )->getMesh();

			float x = cursorPosition.x + rep->bearingU;
			float y = cursorPosition.y - rep->bearingV;

			if( doKerning && lastRep )
				x += font->getKerning( rep, lastRep ); 

			idx = layer->getVertexCount();

			//assign vertex positions and uv coordinates
			layer->vertex( x, y );
			layer->uv( rep->uvPos.x, rep->uvPos.y + rep->uvHeight );

 			layer->vertex( x + rep->widthRatio, y );
 			layer->uv( rep->uvPos.x + rep->uvWidth, rep->uvPos.y + rep->uvHeight );
 
 			layer->vertex( x, y + rep->heightRatio );
 			layer->uv( rep->uvPos.x, rep->uvPos.y );
 
 			layer->vertex( x + rep->widthRatio, y + rep->heightRatio );
 			layer->uv( rep->uvPos.x + rep->uvWidth, rep->uvPos.y );

			layer->triangle( idx, idx+1, idx+2 );
			layer->triangle( idx+1, idx+3, idx+2 );

			//now move to the next character
			cursorPosition.x += rep->advance + charSpacing;

			lastRep = rep;
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

	//push any active layer on the GPU
	_endLayers();

	changed = false;

	return true;
}

void TextArea::_centerLastLine( uint startingAt, float size )
{
	if( mesh->getVertexCount() == 0 )
		return;
	
	float halfWidth = size * 0.5f;
	
	for( uint i = startingAt; i < mesh->getVertexCount(); ++i )
		*(mesh->_getVertex( i )) -= halfWidth;		
}

///create a mesh to be used for text
Mesh* TextArea::_createMesh()
{
	Mesh * mesh = new Mesh();
	mesh->setDynamic( true );
	mesh->setTriangleMode( Mesh::TM_LIST );
	mesh->setVertexFieldEnabled( Mesh::VF_POSITION2D );
	mesh->setVertexFieldEnabled( Mesh::VF_UV );
	
	return mesh;
}
