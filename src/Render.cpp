#include "stdafx.h"

#include "Render.h"

#include "Renderable.h"
#include "TextArea.h"
#include "Platform.h"
#include "Viewport.h"
#include "Light.h"
#include "Mesh.h"
#include "Model.h"
#include "AnimatedQuad.h"

#include "Game.h"

using namespace Dojo;

Render::Render( uint w, uint h, Orientation deviceOr ) :
frameStarted( false ),
viewport( NULL ),
valid( true ),
width( w ),
height( h ),
renderOrientation( DO_LANDSCAPE_RIGHT ),
deviceOrientation( deviceOr ),
currentLayer( NULL ),
frameVertexCount(0),
frameTriCount(0),
frameBatchCount(0),
backLayer( NULL )
{
	platform = Platform::getSingleton();	

	DEBUG_MESSAGE( "Creating OpenGL context...");
	DEBUG_MESSAGE ("querying GL info... ");
	DEBUG_MESSAGE ("vendor: " << glGetString (GL_VENDOR));
	DEBUG_MESSAGE ("renderer: " << glGetString (GL_RENDERER));
	DEBUG_MESSAGE ("version: OpenGL " << glGetString (GL_VERSION));
	
	//clean errors
	DEBUG_ASSERT( glGetError() == 0 );
	glGetError();
	
	glEnable( GL_RESCALE_NORMAL );
	glEnable( GL_NORMALIZE );
	glEnable( GL_CULL_FACE );
	
	glEnable( GL_LIGHTING );

	glCullFace( GL_BACK );
	
	glShadeModel( GL_SMOOTH );

	//default status for blending
	glEnable( GL_BLEND );	
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	//HACK
	//glEnable( GL_COLOR_MATERIAL );
	
	//on IOS this is default and the command is not supported
#ifndef PLATFORM_IOS
	//glColorMaterial( GL_FRONT, GL_DIFFUSE );
#endif
	
	float white[] = {1,1,1,1};
	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, white );
	
	
#ifdef DOJO_GAMMA_CORRECTION_ENABLED
	glEnable( GL_FRAMEBUFFER_SRGB );
#endif
	
	//projection is always the same
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();		

	//always active!
	glEnableClientState(GL_VERTEX_ARRAY);

	currentRenderState = firstRenderState = new RenderState();

	///create the back layer
	backLayer = new Layer();
	backLayer->add( NULL ); //add a dummy element

	setInterfaceOrientation( platform->getGame()->getNativeOrientation() );
	
	nativeToScreenRatio = (float)viewportHeight / (float)platform->getGame()->getNativeHeight();
	
	setDefaultAmbient( Color::BLACK );
}

Render::~Render()
{
	SAFE_DELETE( firstRenderState );
	SAFE_DELETE( backLayer );

	clearLayers();
}

void Render::setWireframe( bool wireframe )
{
#ifndef PLATFORM_IOS
	glPolygonMode( GL_FRONT_AND_BACK, (wireframe) ? GL_LINE : GL_FILL );
#else
	DEBUG_ASSERT( !"WIREFRAME IS NOT SUPPORTED ON OPENGLES AND IS A DEBUG ONLY FEATURE" );
#endif
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

bool Render::hasLayer( int layerID )
{
	LayerList* layerList = &positiveLayers;
	if( layerID < 0 )
	{
		layerID = -layerID - 1; //flip and shift by 1
		layerList = &negativeLayers;
	}
	
	return layerList->size() > abs( layerID );
}

void Render::addRenderable( Renderable* s, int layerID )
{				
	//get the needed layer	
	Layer* layer = getLayer( layerID );
	
#ifndef DOJO_FORCE_WHOLE_RENDERSTATE_COMMIT
	
	//insert this object in the place where the distances from its neighbours are a minimum.	
	uint bestIndex = 0;
	uint bestDistanceSum = 0xffffffff;
	
	uint distance;
	uint lastDistance = firstRenderState->getDistance( s );
	for( int i = 0; i < layer->size(); ++i )
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
#else
	
	s->_notifyRenderInfo( this, layerID, layer->size() );
	
	//append at the end
	layer->add( s );    
	
#endif 
}

void Render::removeRenderable( Renderable* s )
{	
	if( hasLayer( s->getLayer() ) )
		getLayer( s->getLayer() )->remove( s );
	
	s->_notifyRenderInfo( NULL, 0, 0 );

	if( s == currentRenderState )
	{
		//firstRenderState->commitChanges( currentRenderState );
		currentRenderState = firstRenderState;
	}
}

void Render::setViewport( Viewport* v )		
{	
	DEBUG_ASSERT( v );
	
	viewport = v;
}	

void Render::setInterfaceOrientation( Orientation o )		
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
	
	//compute matrix
	mRenderRotation = glm::mat4_cast( Quaternion( Vector( 0,0, Math::toRadian( renderRotation )  ) ) );
}

void Render::startFrame()
{	
	DEBUG_ASSERT( !frameStarted );
	DEBUG_ASSERT( viewport );

	platform->acquireContext();
	
	frameVertexCount = frameTriCount = frameBatchCount = 0;
						
	glViewport( 0, 0, width, height );
		
	//clear the viewport
	glClearColor( 
				 viewport->getClearColor().r, 
				 viewport->getClearColor().g, 
				 viewport->getClearColor().b, 
				 viewport->getClearColor().a );
	
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
				
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
	
	frameStarted = true;
	
	//draw the backdrop
	Renderable* backdrop = viewport->getBackgroundSprite();
	if( backdrop )
	{
		backLayer->at(0) = backdrop;
		renderLayer( backLayer );
	}
	
}

void Render::renderElement( Renderable* s )
{
	DEBUG_ASSERT( frameStarted );
	DEBUG_ASSERT( viewport );

	//if prepare fails, the object can't be rendered
	if( !s->prepare( viewportPixelRatio ) )
		return;

	frameVertexCount += s->getMesh()->getVertexCount();
	frameTriCount += s->getMesh()->getTriangleCount();
	//each renderable is a single batch
	++frameBatchCount;
	
	//change the renderstate
	s->commitChanges( currentRenderState );

	currentRenderState = s;
	
	//clone the view matrix on the top of the stack		
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf( glm::value_ptr( mCurrentViewProj * s->getWorldTransform() ) );
		
	//HACK
#ifndef PLATFORM_IOS
	glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT );
#endif
	glEnable( GL_COLOR_MATERIAL );
	
	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, (float*)(&s->color) );
		
	Mesh* m = currentRenderState->getMesh();

	GLenum mode;
	switch( m->getTriangleMode() )
	{
		case Mesh::TM_LIST:         mode = GL_TRIANGLES;        break;
		case Mesh::TM_STRIP:    	mode = GL_TRIANGLE_STRIP;   break;
		case Mesh::TM_LINE_STRIP:   mode = GL_LINE_STRIP;       break;
	}

	if( !m->isIndexed() )
		glDrawArrays( mode, 0, m->getVertexCount() );
	else
		glDrawElements( mode, m->getIndexCount(), m->getIndexGLType(), 0 );  //on OpenGLES, we have max 65536 indices!!!
}

void Render::endFrame()
{			
	DEBUG_ASSERT( frameStarted );
	
	platform->present();
	
	frameStarted = false;
}

/*
void Render::_setupFrustumProjection()
{
	DEBUG_ASSERT( viewport );

	//compute frustum
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	glRotatef( renderRotation, 0,0,1 );
	
	float f = (renderRotation == 0 || renderRotation == 180 ) ? (float)width/(float)height : (float)height/(float)width;
	
	_gluPerspectiveClone( 
		viewport->getVFOV(), 
		f, 
		viewport->getZNear(), 
		viewport->getZFar() );

	glGetFloatv( GL_PROJECTION_MATRIX, frustumProj );

	//add camera orientation to the modelview
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	Vector center = viewport->getWorldPosition( Vector::UNIT_Z );
	Vector up = viewport->getWorldPosition( Vector::UNIT_Y ) - viewport->getWorldPosition();

	_gluLookAtClone( 
		viewport->getWorldPosition().x, 
		viewport->getWorldPosition().y, 
		viewport->getWorldPosition().z,
		center.x,
		center.y,
		center.z,
		up.x,
		up.y,
		up.z );

	glGetFloatv( GL_MODELVIEW_MATRIX, frustumView );
}*/

void Render::renderLayer( Layer* list )
{
	if( !list->size() || !list->visible )
		return;
	
#ifdef DOJO_WIREFRAME_AVAILABLE
	setWireframe( list->wireframe );
#endif

	//make state changes
	if( list->depthCheck )	glEnable( GL_DEPTH_TEST );
	else					glDisable( GL_DEPTH_TEST );

	mCurrentViewProj =  mRenderRotation * (list->projectionOff ? viewport->getViewProjOrtho() : viewport->getViewProjFrustum());
	
	//we don't want different layers to be depth-checked together?
	if( list->depthClear )
		glClear( GL_DEPTH_BUFFER_BIT );

	currentLayer = list;
	
	if( list->lightingOn )
	{		
		//enable or disable lights - TODO no need to do this each time, use an assigned slot system.
		int i = 0;
		for( ; i < lights.size(); ++i )
		{
			lights[i]->bind( i, mCurrentViewProj );
			
			if( !lights[i]->hasAmbient() )
				glLightfv( GL_LIGHT0 + i, GL_AMBIENT, (float*)&defaultAmbient );
		}
	}
	else
	{		
		for( int i = 0; i < lights.size(); ++i )
			glDisable( GL_LIGHT0 + i );
	}
	
	Renderable* s;

	//2D layer
	if( list->projectionOff )
	{
		for( int i = 0; i < list->size(); ++i )
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
		for( int i = 0; i < list->size(); ++i )
		{
			s = list->at(i);

			s->_notifyCulled( !viewport->isContainedInFrustum( s ) );
			
			if( s->isVisible() && s->isInView() )
				renderElement( s );
		}
	}
}

