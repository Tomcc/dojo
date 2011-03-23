#include "stdafx.h"

#include "Render.h"

#include "Renderable.h"
#include "TextArea.h"
#include "Platform.h"
#include "Viewport.h"

#include "Mesh.h"
#include "Model.h"

#include "Game.h"

using namespace Dojo;

Render::Render( uint w, uint h, uint dps, RenderOrientation deviceOr ) :
frameStarted( false ),
viewport( NULL ),
valid( true ),
width( w ),
height( h ),
devicePixelScale( (float)dps ),
renderOrientation( RO_LANDSCAPE_RIGHT ),
deviceOrientation( deviceOr ),
currentLayer( NULL ),
frameVertexCount(0),
frameTriCount(0),
frameBatchCount(0)
{	
	DEBUG_ASSERT( deviceOrientation <= RO_LANDSCAPE_RIGHT );

	platform = Platform::getSingleton();
			
	//gles settings
	glActiveTexture( GL_TEXTURE0 );		glEnable( GL_TEXTURE_2D );
	glActiveTexture( GL_TEXTURE1 );		glEnable( GL_TEXTURE_2D );
	glActiveTexture( GL_TEXTURE2 );		glEnable( GL_TEXTURE_2D );
	glActiveTexture( GL_TEXTURE3 );		glEnable( GL_TEXTURE_2D );
	glActiveTexture( GL_TEXTURE4 );		glEnable( GL_TEXTURE_2D );
	glActiveTexture( GL_TEXTURE5 );		glEnable( GL_TEXTURE_2D );
	glActiveTexture( GL_TEXTURE6 );		glEnable( GL_TEXTURE_2D );
	glActiveTexture( GL_TEXTURE7 );		glEnable( GL_TEXTURE_2D );

	glEnable( GL_BLEND );	

	glEnable( GL_LIGHTING );
	glEnable( GL_RESCALE_NORMAL );
	glEnable( GL_NORMALIZE );
	glEnable( GL_VERTEX_ARRAY );
	glEnable( GL_CULL_FACE );

	glCullFace( GL_BACK );
	
	glShadeModel( GL_SMOOTH );
	
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glEnable( GL_COLOR_MATERIAL );
	glColorMaterial( GL_FRONT, GL_DIFFUSE );
		
	//projection is always the same
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();		

	//always active!
	glEnableClientState(GL_VERTEX_ARRAY);

	currentRenderState = firstRenderState = new RenderState();

	setInterfaceOrientation( platform->getGame()->getNativeOrientation() );
	
	nativeToScreenRatio = (float)viewportHeight / (float)platform->getGame()->getNativeHeight();
}

Render::~Render()
{
	delete firstRenderState;
}

Render::Layer* Render::getLayer( int layerID )
{	
	LayerList* layerList = &positiveLayers;
	if( layerID < 0 )
	{
		layerID = -layerID - 1; //flip and shift by 1
		layerList = &negativeLayers;
	}
	
	//allocate the needed layers if layerID > layer size
	while( layerList->size() <= layerID )
		layerList->add( new Layer() );	

	if( !currentLayer ) //first layer!
		currentLayer = layerList->at( layerID );

	//get the needed layer	
	return layerList->at( layerID );
}

void Render::addRenderable( Renderable* s, int layerID )
{				
	//get the needed layer	
	Layer* layer = getLayer( layerID );

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
	
	static float orientations[] = 	{ 0, 180, 90, -90 };
	
	renderRotation = orientations[ (uint)renderOrientation ] + orientations[ (uint)deviceOrientation ];
	
	//swap height and width values used in-game if the render has been rotated
	if( renderRotation == 0 || renderRotation == 180 )
	{
		viewportWidth = width;
		viewportHeight = height;
	}
	else 
	{
		viewportWidth = height;
		viewportHeight = width;
	}


	//put a fresh identity matrix over it
	glPushMatrix();
	glLoadIdentity();
}

void Render::startFrame()
{	
	DEBUG_ASSERT( !frameStarted );
	DEBUG_ASSERT( viewport );

	frameVertexCount = frameTriCount = frameBatchCount = 0;
	
	//platform->acquireContext();
					
	glViewport( 0, 0, width, height );
	
	//clear the viewport
	glClearColor( 
				 viewport->getClearColor().r, 
				 viewport->getClearColor().g, 
				 viewport->getClearColor().b, 
				 viewport->getClearColor().a );
	
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
	//load view matrix on top
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();					
	
	//rotate to balance interface orientation
	glRotatef( renderRotation, 0,0,1 );
			
	//translate
	glTranslatef( 
				 -viewport->getWorldPosition().x,
				 -viewport->getWorldPosition().y, 
				 -viewport->getWorldPosition().z );	
	
	if( renderRotation == 0 || renderRotation == 180 )
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

	viewportPixelRatio *= nativeToScreenRatio;
	
	//HACK - uncomment to get proportional pixel scale across resolutions
	//viewportPixelRatio *= devicePixelScale;
	
	frameStarted = true;
	
	//render the background if needed
	Renderable* bg = viewport->getBackgroundSprite();
	
	if( bg )
	{		
		glDisable( GL_DEPTH_TEST );
		glDisable( GL_LIGHTING );

		renderElement( bg );
	}
}

void Render::renderElement( Renderable* s )
{
	DEBUG_ASSERT( frameStarted );
	DEBUG_ASSERT( viewport );

	frameVertexCount += s->getMesh()->getVertexCount();
	frameTriCount += s->getMesh()->getTriangleCount();
	//each renderable is a single batch
	++frameBatchCount;
	
	s->prepare( viewportPixelRatio );
	
	//change the renderstate
	s->commitChanges( currentRenderState );

	currentRenderState = s;
	
	//clone the view matrix on the top of the stack		
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();	
	
	//move
	glTranslatef( 
				 s->getWorldPosition().x,
				 s->getWorldPosition().y,
				 s->getWorldPosition().z );
	
	//rotate
	glRotatef( s->getWorldRotation().x, 1,0,0 );
	glRotatef( s->getWorldRotation().y, 0,1,0 );
	glRotatef( s->getWorldRotation().z, 0,0,1 );
	
	//and then scale
	glScalef( 
			 s->scale.x,
			 s->scale.y, 
			 s->scale.z );
	
	Mesh* m = currentRenderState->getMesh();

	GLenum mode = (m->getTriangleMode() == Mesh::TM_STRIP) ? GL_TRIANGLE_STRIP : GL_TRIANGLES;

	if( !m->isIndexed() )
		glDrawArrays( mode, 0, m->getVertexCount() );
	else
		glDrawElements( mode, m->getIndexCount(), GL_UNSIGNED_INT, 0 );

	//reset original view on the top of the stack
	glPopMatrix();
}

void Render::endFrame()
{			
	DEBUG_ASSERT( frameStarted );
	
	platform->present();
	
	frameStarted = false;
}

void Render::_setOrthoProjection()
{	
	DEBUG_ASSERT( viewport );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 
		-viewport->getHalfSize().x, 
		viewport->getHalfSize().x,
		-viewport->getHalfSize().y,
		viewport->getHalfSize().y,
		-1000,
		1000 );
}

void Render::_setFrustumProjection()
{
	DEBUG_ASSERT( viewport );

	//compute frustum
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 
		viewport->getVFOV(), 
		(float)width/(float)height, 
		viewport->getZNear(), 
		viewport->getZFar() );

	glRotatef( viewport->getWorldRotation().x, 1,0,0 );
	glRotatef( viewport->getWorldRotation().y, 0,1,0 );
	glRotatef( viewport->getWorldRotation().z, 0,0,1 );
}

void Render::renderLayer( Layer* list )
{
	if( !list->size() )
		return;

	//make state changes
	if( list->depthCheck )	glEnable( GL_DEPTH_TEST );
	else					glDisable( GL_DEPTH_TEST );


	if( list->lightingOn )	glEnable( GL_LIGHTING );
	else					glDisable( GL_LIGHTING );

	if( list->projectionOff )	_setOrthoProjection();
	else						_setFrustumProjection();

	//we don't want different layers to be depth-checked together
	glClear( GL_DEPTH_BUFFER_BIT );

	currentLayer = list;

	Renderable* s;

	//2D layer
	if( list->projectionOff )
	{
		for( uint i = 0; i < list->size(); ++i )
		{
			s = list->at(i);
			
			//HACK use some 2D culling
			if( s->isVisible() )
				renderElement( s );
		}
	}

	//3D layer
	else
	{
		for( uint i = 0; i < list->size(); ++i )
		{
			s = list->at(i);

			if( s->isVisible() && viewport->isContainedInFrustum( s ) )
				renderElement( s );
		}
	}
}

