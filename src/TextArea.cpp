#include "stdafx.h"

#include "TextArea.h"
#include "Game.h"
#include "GameState.h"
#include "Mesh.h"
#include "Viewport.h"

using namespace Dojo;

TextArea::TextArea( Object* l, 
		 const String& fontSetName, 
		 const Vector& pos, 
		 bool center,
		 const Vector& bounds) :
Renderable( l, pos ),
fontName( fontSetName ),
interline( 0.2f ),
maxLineLenght( 0xfffffff ),
centered( center ),
pixelScale( 1,1 ),
currentLineLength( 0 ),
lastSpace( 0 ),
visibleCharsNumber( 0xfffffff )
{		
	setSize( bounds );
	cullMode = CM_DISABLED;

	font = gameState->getFont( fontName );

	DEBUG_ASSERT_INFO( font, "Cannot find the required font for a TextArea", "fontName = " + fontName );

	charSpacing = font->getSpacing();
	spaceWidth = font->getCharacter(' ')->advance;
	
	//not visible until prepared!
	scale = 0;

	//init
	clearText();
}

TextArea::~TextArea()
{			
	clearText();

	if( mesh )
	{
		if( mesh->isLoaded() )
			mesh->onUnload();				
		SAFE_DELETE( mesh );
	}

	_destroyLayers();
}

void TextArea::clearText()
{
	characters.clear();

	content.clear();

	cursorPosition = Vector::ZERO;

	setSize(0,0);

	changed = true;
	
	visibleCharsNumber = 0xfffffff;
	currentLineLength = 0;
	lastSpace = 0;
}

void TextArea::setMaxLineLength( int l )
{		
	//HACK PAZZESCOH
	maxLineLenght = (int)(l * ((float)gameState->getGame()->getNativeWidth() / (float)640 ));
}

void TextArea::addText( const String& text )
{
	content += text;

	Font::Character* currentChar;
	unichar c;

	//parse and setup characters
	for( int i = 0; i < (int)text.size(); ++i )
	{
		c = text[i];

		currentChar = font->getCharacter( c );
		characters.emplace( currentChar );

		currentLineLength += currentChar->pixelWidth;

		if( c == ' ' || c == '\t' )
			lastSpace = characters.size()-1;

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

void TextArea::addText( int n, char paddingChar, int digits )
{
	//TODO String already does this! remove

	String number( n );

	//stay in the digit budget?
	if( paddingChar != 0 )
	{				
		//forget most significative digits
		if( (int)number.size() > digits )
			number = number.substr( number.size() - digits );

		//pad to fill
		else if ((int) number.size() < digits)
		{
			String padding;

			for( size_t i = 0; i < digits - number.size(); ++i )
				padding += paddingChar;

			number = padding + number;
		}
	}

	addText( number );
}

Renderable* TextArea::_enableLayer( Texture& tex )
{
	if( freeLayers.empty() )
		_pushLayer();
	
	Renderable* r = *freeLayers.begin();
	freeLayers.erase(freeLayers.begin());
	
	r->setVisible( true );
	r->setActive( true );
	r->setTexture( &tex );
	
	r->getMesh()->begin( getLenght() * 2 );
	
	busyLayers.emplace( r );
	
	return r;
}

void TextArea::_endLayers()
{
	for( size_t i = 0; i < busyLayers.size(); ++i )
		busyLayers[i]->getMesh()->end();
	
	//also end this
	if( mesh->isEditing() )
		mesh->end();
}

///Free any created layer			
void TextArea::_hideLayers()
{
	for( size_t i = 0; i < busyLayers.size(); ++i )
	{
		Renderable* l = busyLayers[i];
		
		l->setVisible( false );
		l->setActive( false );
		
		freeLayers.emplace( l );
	}
	
	actualCharacters = 0;
	busyLayers.clear();
}

void TextArea::_destroyLayer( Renderable& r )
{
	if( &r == this )  //do not delete the TA itself, even if it is a layer
		return;
	
	delete r.getMesh();
	
	busyLayers.erase(&r);
	freeLayers.erase(&r);
	removeChild(r);
}


void TextArea::_prepare() {
	//not changed
	if( !changed )
		return;

	//no characters to show
	if( !visibleCharsNumber || getLenght() == 0 )
		return;
    
    //setup the aspect ratio
    gameState->getViewport()->makeScreenSize( screenSize, font->getFontWidth(), font->getFontHeight() );
    
    pixelScale.z = 1;
    screenSize = screenSize.mulComponents( pixelScale );
    scale = screenSize;

    //render the font
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
	for( size_t i = 0; i < visibleCharsNumber && i < characters.size(); ++i )
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
			cursorPosition.x += spaceWidth*4; //TODO align to nearest tab
			lastRep = NULL;
		}
		else if( rep->character == ' ' )
		{
			cursorPosition.x += spaceWidth;
			lastRep = NULL;
		}
		else	//real character
		{
			Mesh* layer = _getLayer( *rep->getTexture() )->getMesh();

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
			++actualCharacters;
		}	

		//update size to contain cursorPos to the longest line
		if( cursorPosition.x > newSize.x )
			newSize.x = cursorPosition.x;
	}

	//if centered move every character of this line along x of 1/2 size
	if( centered )
		_centerLastLine( lastLineVertexID, cursorPosition.x );
	//push any active layer on the GPU
	_endLayers();
   
	//find real mesh bounds
	mLayersLowerBound = mesh->getMin();
	mLayersUpperBound = mesh->getMax();
	
	for( size_t i = 0; i < busyLayers.size(); ++i )
	{
		mLayersUpperBound = Math::max( mLayersUpperBound, busyLayers[i]->getMesh()->getMax() );
		mLayersLowerBound = Math::min( mLayersLowerBound, busyLayers[i]->getMesh()->getMin() );
	}

	setSize( mLayersUpperBound - mLayersLowerBound );
   
	changed = false;
}

void TextArea::_destroyLayers() {
	for (auto&& l : busyLayers)
		_destroyLayer(*l);

	for (auto&& l : freeLayers)
		_destroyLayer(*l);
}

void TextArea::_centerLastLine( int startingAt, float size )
{
	if( mesh->getVertexCount() == 0 )
		return;
	
	float halfWidth = size * 0.5f;
	
	for( Mesh::IndexType i = startingAt; i < mesh->getVertexCount(); ++i )
		mesh->getVertex( i ).x -= halfWidth;		
}

///create a mesh to be used for text
Mesh* TextArea::_createMesh()
{
	Mesh * mesh = new Mesh();
	mesh->setDynamic( true );
	mesh->setVertexFields({ VertexField::Position2D, VertexField::UV0 });
	mesh->setTriangleMode( TriangleMode::TriangleList );
	
	return mesh;
}

void TextArea::_pushLayer() 
{
	auto r = make_unique<Renderable>( gameState, Vector::ZERO );
	r->scale = scale;
	r->setMesh( _createMesh() );
	r->setVisible( false );
	r->setActive( false );

	freeLayers.emplace(r.get());
	addChild( std::move(r), getLayer() );
}

Renderable* TextArea::_getLayer(Texture& tex) 
{
	//find this layer in the already assigned, or get new
	for (size_t i = 0; i < busyLayers.size(); ++i)
	{
		if (busyLayers[i]->getTexture() == &tex)
			return busyLayers[i];
	}

	//does not exist, hit a free one
	return _enableLayer(tex);
}

void TextArea::onAction(float dt)
{
	_prepare();

	{
		Object::onAction(dt);

		advanceFade(dt);
	}
	
	_updateWorldAABB( mLayersLowerBound, mLayersUpperBound );
}
