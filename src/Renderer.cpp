#include "stdafx.h"

#include "Renderer.h"

#include "Renderable.h"
#include "TextArea.h"
#include "Platform.h"
#include "Viewport.h"
#include "Light.h"
#include "Mesh.h"
#include "AnimatedQuad.h"
#include "Shader.h"

#include "Game.h"
#include "Texture.h"

using namespace Dojo;

Renderer::Renderer( int w, int h, Orientation deviceOr ) :
frameStarted( false ),
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
	DEBUG_MESSAGE( "Creating OpenGL context...");
	DEBUG_MESSAGE ("querying GL info... ");
	DEBUG_MESSAGE ("vendor: " + String( (const char*)glGetString (GL_VENDOR)));
	DEBUG_MESSAGE ("renderer: " + String( (const char*)glGetString (GL_RENDERER)));
	DEBUG_MESSAGE ("version: OpenGL " + String( (const char*)glGetString (GL_VERSION)));
	
	//clean errors (some drivers leave errors on the stack)
	CHECK_GL_ERROR;
	CHECK_GL_ERROR;
	
	glEnable( GL_RESCALE_NORMAL );
	glEnable( GL_NORMALIZE );
	glEnable( GL_CULL_FACE );

	glCullFace( GL_BACK );
	
	glShadeModel( GL_SMOOTH );

	//default status for blending
	glEnable( GL_BLEND );	
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	glEnable( GL_COLOR_MATERIAL );
	
	//on IOS this is default and the command is not supported
#ifndef USING_OPENGLES
	glColorMaterial( GL_FRONT, GL_DIFFUSE );
#endif	
	
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

	setInterfaceOrientation( Platform::singleton().getGame().getNativeOrientation() );
	
	setDefaultAmbient( Color::BLACK );

	CHECK_GL_ERROR;
}

Renderer::~Renderer()
{
	SAFE_DELETE( firstRenderState );
	SAFE_DELETE( backLayer );

	clearLayers();
}

Renderer::Layer* Renderer::getLayer( int layerID )
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

bool Renderer::hasLayer( int layerID )
{
	LayerList* layerList = &positiveLayers;
	if( layerID < 0 )
	{
		layerID = -layerID - 1; //flip and shift by 1
		layerList = &negativeLayers;
	}
	
	return layerList->size() > abs( layerID );
}

void Renderer::addRenderable( Renderable* s, int layerID )
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

void Renderer::removeRenderable( Renderable* s )
{	
	if( hasLayer( s->getLayer() ) )
	{
		getLayer( s->getLayer() )->remove( s );
		s->_notifyRenderInfo( NULL, 0, 0 );
	}

	if( s == currentRenderState )
	{
		//firstRenderState->commitChanges( currentRenderState );
		currentRenderState = firstRenderState;
	}
}

void Dojo::Renderer::removeAllRenderables() {
	for (int i = 0; i < negativeLayers.size(); ++i)
		negativeLayers.at(i)->clear();

	for (int i = 0; i < positiveLayers.size(); ++i)
		positiveLayers.at(i)->clear();
}

void Dojo::Renderer::removeViewport(Viewport* v) {
	mViewportList.remove(v);
}

void Dojo::Renderer::removeAllViewports() {
	mViewportList.clear();
}

void Dojo::Renderer::addLight(Light* l) {
	DEBUG_ASSERT(l, "addLight: null light passed");
	DEBUG_ASSERT(lights.size() < RENDER_MAX_LIGHTS, "addLight: Cannot add this light as it is past the supported light number (RENDER_MAX_LIGHTS)");

	lights.add(l);
}

void Dojo::Renderer::removeLight(Light* l) {
	DEBUG_ASSERT(l, "removeLight: null light passed");

	lights.remove(l);

	//remove removes always the last element in the list - just disable the last index now
	glDisable(GL_LIGHT0 + lights.size());
}

void Dojo::Renderer::clearLayers() {
	for (int i = 0; i < negativeLayers.size(); ++i)
		SAFE_DELETE(negativeLayers[i]);

	negativeLayers.clear();

	for (int i = 0; i < positiveLayers.size(); ++i)
		SAFE_DELETE(positiveLayers[i]);

	positiveLayers.clear();
}

void Renderer::setDefaultAmbient(const Color& a) {
	defaultAmbient = a;
	defaultAmbient.a = 1;
}

void Renderer::addViewport( Viewport* v )
{
	DEBUG_ASSERT( v, "cannot add a null vieport" );

	mViewportList.add( v );
}

void Renderer::setInterfaceOrientation( Orientation o )		
{	
	renderOrientation = o;
	
	static float orientations[] = 	{ 0, 180, 90, -90 };
	
	renderRotation = orientations[ (int)renderOrientation ] + orientations[ (int)deviceOrientation ];
	
	//compute matrix
	mRenderRotation = glm::mat4_cast( Quaternion( Vector( 0,0, Math::toRadian( renderRotation )  ) ) );
}

void Renderer::renderElement( Viewport* viewport, Renderable* s )
{
	DEBUG_ASSERT( frameStarted, "Tried to render an element but the frame wasn't started" );
	DEBUG_ASSERT(viewport, "Rendering requires a Viewport to be set");
	DEBUG_ASSERT(s->getMesh()->isLoaded(), "Rendering with a mesh with no GPU data!");
	DEBUG_ASSERT(s->getMesh()->getVertexCount() > 0, "Rendering a mesh with no vertices");

#ifndef PUBLISH
	frameVertexCount += s->getMesh()->getVertexCount();
	frameTriCount += s->getMesh()->getPrimitiveCount();

	//each renderable is a single batch
	++frameBatchCount;
#endif // !PUBLISH
	
	//change the renderstate
	currentRenderState = s;
	
	currentState.world = s->getWorldTransform();
	currentState.worldView = currentState.view * currentState.world;

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf( glm::value_ptr( currentState.worldView ) );
	
#ifdef DOJO_SHADERS_AVAILABLE
	if( s->getShader() )
	{
		currentState.worldViewProjection = currentState.projection * currentState.worldView;
		s->getShader()->use( s );
	}
	else
		glUseProgram( 0 );
#endif
		
	//I'm not sure this actually makes sense
#ifndef USING_OPENGLES
	glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT );
#endif
	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, (float*)(&s->color) );
	
	glEnable( GL_COLOR_MATERIAL );

	s->commitChanges( currentRenderState );

	static const GLenum glModeMap[] = {
		GL_TRIANGLE_STRIP, //TriangleStrip,
		GL_TRIANGLES, //TriangleList,
		GL_LINE_STRIP, //LineStrip,
		GL_LINES, //LineList
		GL_POINTS
	};

	Mesh* m = currentRenderState->getMesh();
	GLenum mode = glModeMap[(byte)m->getTriangleMode()];

	if( !m->isIndexed() )
		glDrawArrays( mode, 0, m->getVertexCount() );
	else {
		DEBUG_ASSERT(m->getIndexCount() > 0, "Rendering an indexed mesh with no indices");
		glDrawElements(mode, m->getIndexCount(), m->getIndexGLType(), 0);  //on OpenGLES, we have max 65536 indices!!!
	}

#ifndef DOJO_DISABLE_VAOS
	glBindVertexArray( 0 );
#endif

#ifdef DOJO_SHADERS_AVAILABLE
	//HACK //TODO remove fixed function pipeline (it breaks if generic arrays are set)
	if( s->getShader() )
	{
		for( auto& attr : s->getShader()->getAttributes() )
			glDisableVertexAttribArray( attr.second.location );
	}
#endif
}

void Renderer::renderLayer( Viewport* viewport, Layer* list )
{
	if( !list->size() || !list->visible )
		return;
	
#ifdef DOJO_WIREFRAME_AVAILABLE
	glPolygonMode(GL_FRONT_AND_BACK, list->wireframe ? GL_LINE : GL_FILL);
#endif

	//make state changes
	if( list->depthCheck )	glEnable( GL_DEPTH_TEST );
	else					glDisable( GL_DEPTH_TEST );

	//set projection state
	currentState.projection = mRenderRotation * (list->projectionOff ? viewport->getOrthoProjectionTransform() : viewport->getPerspectiveProjectionTransform());
	
	glMatrixMode( GL_PROJECTION );
	glLoadMatrixf( glm::value_ptr( currentState.projection ) );
	
	//we don't want different layers to be depth-checked together?
	if( list->depthClear )
		glClear( GL_DEPTH_BUFFER_BIT );

	currentLayer = list;
	
	if( list->lightingOn )
	{		
		glEnable( GL_LIGHTING );
		//enable or disable lights - TODO no need to do this each time, use an assigned slot system.
		int i = 0;
		for( ; i < lights.size(); ++i )
		{
			lights[i]->bind( i, currentState.view );
			
			if( !lights[i]->hasAmbient() )
				glLightfv( GL_LIGHT0 + i, GL_AMBIENT, (float*)&defaultAmbient );
		}
	}
	else
	{
		glDisable( GL_LIGHTING );

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
			if( s->canBeRendered() )
				renderElement( viewport, s );
		}
	}

	//3D layer
	else
	{
		for( int i = 0; i < list->size(); ++i )
		{
			s = list->at(i);

			s->_notifyCulled( !viewport->isContainedInFrustum( s ) );
			
			if (s->canBeRendered() && s->isInView())
				renderElement( viewport, s );
		}
	}
}

void Renderer::renderViewport( Viewport* viewport )
{
	DEBUG_ASSERT( viewport, "Cannot render with a null viewport" );

	Texture* rt = viewport->getRenderTarget();

	if( rt )
		rt->bindAsRenderTarget( true ); //TODO guess if this viewport doesn't render 3D layers to save memory?
	else
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	glFrontFace( rt ? GL_CW : GL_CCW ); //invert vertex winding when inverting the view

    currentState.targetDimension.x = (float)(rt ? rt->getWidth() : width);
    currentState.targetDimension.y = (float)(rt ? rt->getHeight() : height);
    
	glViewport(0, 0, (GLsizei) currentState.targetDimension.x, (GLsizei)currentState.targetDimension.y);

	//clear the viewport
	if (viewport->getClearEnabled()) {
		glClearColor(
			viewport->getClearColor().r,
			viewport->getClearColor().g,
			viewport->getClearColor().b,
			viewport->getClearColor().a);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	currentState.view = viewport->getViewTransform();
	currentState.viewDirection = viewport->getWorldDirection();

	frameStarted = true;

	if( viewport->getVisibleLayers().empty() ) //use the custom layer ordering/visibility
	{
		//first render from the most negative to -1
		if( negativeLayers.size() > 0 )
		{	
			for( int i = negativeLayers.size()-1; i >= 0; --i )
				renderLayer( viewport, negativeLayers[i] );
		}

		//then from 0 to the most positive
		for( int i = 0; i < positiveLayers.size(); ++i )
			renderLayer( viewport, positiveLayers[i] );
	}
	else  //using the default layer ordering/visibility
	{
		for( auto& layer : viewport->getVisibleLayers() )
			renderLayer(viewport, getLayer(layer));
	}
}

void Renderer::render()
{
	DEBUG_ASSERT( !frameStarted, "Tried to start rendering but the frame was already started" );


	frameVertexCount = frameTriCount = frameBatchCount = 0;
	frameStarted = true;

	//render all the viewports
	for( auto viewport : mViewportList )
		renderViewport( viewport );

	frameStarted = false;
}