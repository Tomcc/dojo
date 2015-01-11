#include "stdafx.h"

#include "Renderer.h"

#include "Renderable.h"
#include "TextArea.h"
#include "Platform.h"
#include "Viewport.h"
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
frameBatchCount(0)
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

	setInterfaceOrientation( Platform::singleton().getGame().getNativeOrientation() );
	
	setDefaultAmbient( Color::BLACK );

	CHECK_GL_ERROR;
}

Renderer::~Renderer()
{
	clearLayers();
}

RenderLayer& Renderer::getLayer( RenderLayer::ID layerID )
{	
	auto layerList = &positiveLayers;
	if( layerID < 0 )
	{
		layerID = -layerID - 1; //flip and shift by 1
		layerList = &negativeLayers;
	}
	
	//allocate the needed layers if layerID > layer size
	while( (RenderLayer::ID)layerList->size() <= layerID )
		layerList->emplace_back();	

	if( !currentLayer ) //first layer!
		currentLayer = &(*layerList)[ layerID ];

	//get the needed layer	
	return (*layerList)[layerID];
}

bool Renderer::hasLayer( RenderLayer::ID layerID )
{
	auto layerList = &positiveLayers;
	if( layerID < 0 )
	{
		layerID = -layerID - 1; //flip and shift by 1
		layerList = &negativeLayers;
	}
	
	return (RenderLayer::ID)layerList->size() > abs( layerID );
}

void Renderer::addRenderable( Renderable& s, RenderLayer::ID layerID )
{				
	//get the needed layer	
	RenderLayer& layer = getLayer( layerID );
	
	s._notifyRenderInfo( this, layerID, layer.elements.size() );
	
	//append at the end
	layer.elements.emplace( &s );
}

void Renderer::removeRenderable( Renderable& s )
{	
	getLayer(s.getLayer()).elements.erase(&s);
	s._notifyRenderInfo( NULL, 0, 0 );
}

void Renderer::removeAllRenderables() {
	for (auto& l : negativeLayers)
		l.elements.clear();

	for (auto& l : positiveLayers)
		l.elements.clear();
}

void Renderer::removeViewport(const Viewport& v) {

}

void Renderer::removeAllViewports() {
	viewportList.clear();
}

void Renderer::clearLayers() {
	negativeLayers.clear();
	positiveLayers.clear();
}

void Renderer::setDefaultAmbient(const Color& a) {
	defaultAmbient = a;
	defaultAmbient.a = 1;
}

void Renderer::addViewport( Viewport& v )
{
	viewportList.push_back( &v );
}

void Renderer::setInterfaceOrientation( Orientation o )		
{	
	renderOrientation = o;
	
	static float orientations[] = 	{ 0, 180, 90, -90 };
	
	renderRotation = orientations[ (int)renderOrientation ] + orientations[ (int)deviceOrientation ];
	
	//compute matrix
	mRenderRotation = glm::mat4_cast( Quaternion( Vector( 0,0, Math::toRadian( renderRotation )  ) ) );
}

void Renderer::renderElement( Viewport& viewport, Renderable& elem )
{
	DEBUG_ASSERT( frameStarted, "Tried to render an element but the frame wasn't started" );
	DEBUG_ASSERT(elem.getMesh()->isLoaded(), "Rendering with a mesh with no GPU data!");
	DEBUG_ASSERT(elem.getMesh()->getVertexCount() > 0, "Rendering a mesh with no vertices");

#ifndef PUBLISH
	frameVertexCount += elem.getMesh()->getVertexCount();
	frameTriCount += elem.getMesh()->getPrimitiveCount();

	//each renderable is a single batch
	++frameBatchCount;
#endif // !PUBLISH
	
	currentState.world = elem.getWorldTransform();
	currentState.worldView = currentState.view * currentState.world;

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf( glm::value_ptr( currentState.worldView ) );
	
#ifdef DOJO_SHADERS_AVAILABLE
	if( elem.getShader() )
	{
		currentState.worldViewProjection = currentState.projection * currentState.worldView;
		elem.getShader()->use( elem );
	}
	else
		glUseProgram( 0 );
#endif
		
	//I'm not sure this actually makes sense
#ifndef USING_OPENGLES
	glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT );
#endif
	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, (float*)(&elem.color) );
	
	glEnable( GL_COLOR_MATERIAL );

	elem.commitChanges();

	static const GLenum glModeMap[] = {
		GL_TRIANGLE_STRIP, //TriangleStrip,
		GL_TRIANGLES, //TriangleList,
		GL_LINE_STRIP, //LineStrip,
		GL_LINES, //LineList
		GL_POINTS
	};

	Mesh* m = elem.getMesh();
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
	if( elem.getShader() )
	{
		for( auto& attr : elem.getShader()->getAttributes() )
			glDisableVertexAttribArray( attr.second.location );
	}
#endif
}

bool _cull(const RenderLayer& layer, const Viewport& viewport, const Renderable& r) {
	return layer.orthographic ? viewport.isInViewRect(r) : viewport.isContainedInFrustum(r);
}

void Renderer::renderLayer( Viewport& viewport, const RenderLayer& layer )
{
	if( !layer.elements.size() || !layer.visible )
		return;
	
#ifdef DOJO_WIREFRAME_AVAILABLE
	glPolygonMode(GL_FRONT_AND_BACK, layer.wireframe ? GL_LINE : GL_FILL);
#endif

	//make state changes
	if( layer.depthCheck )	glEnable( GL_DEPTH_TEST );
	else					glDisable( GL_DEPTH_TEST );

	//set projection state
	currentState.projection = mRenderRotation * (layer.orthographic ? viewport.getOrthoProjectionTransform() : viewport.getPerspectiveProjectionTransform());
	
	glMatrixMode( GL_PROJECTION );
	glLoadMatrixf( glm::value_ptr( currentState.projection ) );
	
	//we don't want different layers to be depth-checked together?
	if( layer.depthClear )
		glClear( GL_DEPTH_BUFFER_BIT );

	currentLayer = &layer;

	for (auto& r : layer.elements)
	{
		if( r->canBeRendered() && _cull(layer, viewport, *r))
			renderElement( viewport, *r );
	}
}

void Renderer::renderViewport( Viewport& viewport )
{
	Texture* rt = viewport.getRenderTarget();

	if( rt )
		rt->bindAsRenderTarget( true ); //TODO guess if this viewport doesn't render 3D layers to save memory?
	else
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	glFrontFace( rt ? GL_CW : GL_CCW ); //invert vertex winding when inverting the view

    currentState.targetDimension.x = (float)(rt ? rt->getWidth() : width);
    currentState.targetDimension.y = (float)(rt ? rt->getHeight() : height);
    
	glViewport(0, 0, (GLsizei) currentState.targetDimension.x, (GLsizei)currentState.targetDimension.y);

	//clear the viewport
	if (viewport.getClearEnabled()) {
		glClearColor(
			viewport.getClearColor().r,
			viewport.getClearColor().g,
			viewport.getClearColor().b,
			viewport.getClearColor().a);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	currentState.view = viewport.getViewTransform();
	currentState.viewDirection = viewport.getWorldDirection();

	frameStarted = true;

	if( viewport.getVisibleLayers().empty() ) //using the default layer ordering/visibility
	{
		//first render from the most negative to -1
		for( int i = (int)negativeLayers.size()-1; i >= 0; --i )
			renderLayer( viewport, negativeLayers[i] );

		//then from 0 to the most positive
		for (auto& l : positiveLayers)
			renderLayer( viewport, l );
	}
	else   //use the custom layer ordering/visibility
	{
		for( auto& layer : viewport.getVisibleLayers() )
			renderLayer(viewport, getLayer(layer));
	}
}

void Renderer::render()
{
	DEBUG_ASSERT( !frameStarted, "Tried to start rendering but the frame was already started" );


	frameVertexCount = frameTriCount = frameBatchCount = 0;
	frameStarted = true;

	//render all the viewports
	for( auto& viewport : viewportList )
		renderViewport( *viewport );

	frameStarted = false;
}

