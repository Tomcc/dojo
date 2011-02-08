#include "stdafx.h"

#include "Render.h"

#include "Renderable.h"
#include "TextArea.h"
#include "Platform.h"
#include "Viewport.h"

#include "Mesh.h"
#include "Model.h"

using namespace Dojo;

Render::Render( uint w, uint h, uint dps, RenderOrientation deviceOr ) :
frameStarted( false ),
viewport( NULL ),
valid( true ),
cullingEnabled( true ),
width( w ),
height( h ),
devicePixelScale( (float)dps ),
renderOrientation( RO_LANDSCAPE_LEFT ),
deviceOrientation( deviceOr )
{	
	DEBUG_ASSERT( deviceOrientation <= RO_LANDSCAPE_LEFT );

	platform = Platform::getSingleton();

	// Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
	//HACK - IOS renderizza in un RenderBuffer, mentre Windows renderizza diretto
#ifdef PLATFORM_IOS
	glGenFramebuffers(1, &defaultFramebuffer);
	glGenRenderbuffers(1, &colorRenderbuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
#endif
		
	//gles settings
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );	
	
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		
	glDisable( GL_LIGHTING );
	glDisable( GL_DEPTH_TEST );
		
	//projection is always the same
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();		
					
	//setup object data	
	glEnableClientState(GL_VERTEX_ARRAY);			
	//glEnableClientState(GL_COLOR_ARRAY);			
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);			
	//glEnableClientState(GL_NORMAL_ARRAY);
		
	currentRenderState = firstRenderState = new RenderState();

	setInterfaceOrientation( RO_LANDSCAPE_LEFT );
}

Render::~Render()
{
	delete firstRenderState;
	
	// Tear down GL
	if (defaultFramebuffer)
	{
		glDeleteFramebuffers(1, &defaultFramebuffer);
		defaultFramebuffer = 0;
	}
	
	if (colorRenderbuffer)
	{
		glDeleteRenderbuffers(1, &colorRenderbuffer);
		colorRenderbuffer = 0;
	}
}


bool Render::onResize()
{
	glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
	
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);

	_updateGLViewportDimensions();
	
	return (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE);
}

Render::RenderableList* Render::getLayer( int layerID )
{	
	LayerList* layerList = &positiveLayers;
	if( layerID < 0 )
	{
		layerID = -layerID - 1; //flip and shift by 1
		layerList = &negativeLayers;
	}
	
	//allocate the needed layers if layerID > layer size
	while( layerList->size() <= layerID )
		layerList->add( new RenderableList() );	
	
	//get the needed layer	
	return layerList->at( layerID );
}

void Render::addRenderable( Renderable* s, int layerID )
{				
	//get the needed layer	
	RenderableList* layer = getLayer( layerID );

	//insert this object in the place where the distances from its neighbours are a minimum.	
	uint bestIndex = 0;
	uint bestDistanceSum = 0xffffffff;
	
	uint distance;
	uint lastDistance = firstRenderState->getDistance( s );
	for( uint i = 0; i < layer->size(); ++i )
	{
		distance = layer->at(i)->getDistance( s );
		if( distance + lastDistance < bestDistanceSum )
		{
			bestDistanceSum = distance + lastDistance;
			bestIndex = i;
		}
		
		lastDistance = distance;
	}
		
	s->_notifyRenderInfo( this, layerID, bestIndex );
		
	layer->add( s, bestIndex );
}

void Render::removeRenderable( Renderable* s )
{	
	getLayer( s->getLayer() )->remove( s );
	
	s->_notifyRenderInfo( NULL, 0, 0 );
}

void Render::setViewport( Viewport* v )		
{	
	DEBUG_ASSERT( v );
	
	viewport = v;
}	

void Render::setInterfaceOrientation( RenderOrientation o )		
{	
	renderOrientation = o;
	
	static float orientations[] = 	{ 0, 180, -90, 90 };
	
	renderRotation = orientations[ (uint)renderOrientation ] + orientations[ (uint)deviceOrientation ];

	_updateGLViewportDimensions();
}

void Render::_updateGLViewportDimensions()
{
	viewportWidth = width;
	viewportHeight = height;

	if( renderOrientation == 90.f || renderOrientation == -90.f )
	{
		viewportWidth = height;
		viewportHeight = width;
	}
}

void Render::startFrame()
{	
	DEBUG_ASSERT( !frameStarted );
	DEBUG_ASSERT( viewport );
	
	platform->acquireContext();
				
	// This application only creates a single default framebuffer which is already bound at this point.
	// This call is redundant, but needed if dealing with multiple framebuffers.
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
	
	glViewport( 0, 0, viewportWidth, viewportHeight );
	
	//clear the viewport
	glClearColor( 
				 viewport->getClearColor().r, 
				 viewport->getClearColor().g, 
				 viewport->getClearColor().b, 
				 viewport->getClearColor().a );
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	//load view matrix on top
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();					
	
	//rotate to balance interface orientation
	glRotatef( renderRotation, 0, 0, -1 );
	
	//scale with area and window ratio
	glScalef( 
			 2.f / viewport->getSize().x, 
			 2.f / viewport->getSize().y, 
			 1.f);
		
	//translate
	glTranslatef( 
				 -viewport->position.x,
				 -viewport->position.y, 
				 0.f );		
	
	if( renderOrientation == RO_LANDSCAPE_LEFT || renderOrientation == RO_LANDSCAPE_RIGHT )
	{
		viewportPixelRatio.x = viewport->getSize().x / width;
		viewportPixelRatio.y = viewport->getSize().y / height;
	}
	else 
	{
		//swap
		viewportPixelRatio.x = viewport->getSize().y / width;
		viewportPixelRatio.y = viewport->getSize().x / height;
	}
	
	//HACK - uncomment to get proportional pixel scale across resolutions
	//viewportPixelRatio *= devicePixelScale;
	
	frameStarted = true;
	
	//render the background if needed
	Renderable* bg = viewport->getBackgroundSprite();
	
	if( bg )
	{		
		renderElement( bg );
	}
}

void Render::renderElement( Renderable* s )
{
	DEBUG_ASSERT( frameStarted );
	DEBUG_ASSERT( viewport );
	
	s->prepare( viewportPixelRatio );
	
	//change the renderstate
	s->commitChanges( currentRenderState );
	currentRenderState = s;
	
	//clone the view matrix on the top of the stack		
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();	
	
	//move
	glTranslatef( 
				 s->position.x,
				 s->position.y,
				 s->position.z );
	
	//rotate
	glRotatef( s->spriteRotation, 0, 0, 1 );
	
	//and then scale with the pixel size
	glScalef( 
			 s->scale.x,
			 s->scale.y, 
			 s->scale.z );
	
	Mesh* m = currentRenderState->getMesh();
	switch( m->getTriangleMode() )
	{
		case Mesh::TM_STRIP:
			glDrawArrays(GL_TRIANGLE_STRIP, 0, m->getVertexCount());
			break;
			
		case Mesh::TM_LIST:
			glDrawArrays(GL_TRIANGLES, 0, m->getVertexCount());
			break;
	}	
	
	//reset original view on the top of the stack
	glPopMatrix();
}

void Render::endFrame()
{			
	// This application only creates a single color renderbuffer which is already bound at this point.
	// This call is redundant, but needed if dealing with multiple renderbuffers.
	DEBUG_ASSERT( frameStarted );
	
	glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
	
	platform->present();
	
	frameStarted = false;
}

void Render::renderLayer( RenderableList* list )
{
	Renderable* s;

	for( uint i = 0; i < list->size(); ++i )
	{
		s = list->at(i);
		
		if( s->isVisible() && (!cullingEnabled || s->_canBeRenderedBy( viewport ) ) )
			renderElement( s );
	}
}

