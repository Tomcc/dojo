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

#include "dojo_gl_header.h"

using namespace Dojo;

Renderer::Renderer(int w, int h, Orientation deviceOr) :
	frameStarted(false),
	valid(true),
	width(w),
	height(h),
	renderOrientation(DO_LANDSCAPE_RIGHT),
	deviceOrientation(deviceOr),
	frameVertexCount(0),
	frameTriCount(0),
	frameBatchCount(0),
	submitter(Platform::singleton()) {
	DEBUG_MESSAGE( "Creating OpenGL context...");
	DEBUG_MESSAGE ("querying GL info... ");
	DEBUG_MESSAGE ("vendor: " + utf::string( (const char*)glGetString (GL_VENDOR)));
	DEBUG_MESSAGE ("renderer: " + utf::string( (const char*)glGetString (GL_RENDERER)));
	DEBUG_MESSAGE ("version: OpenGL " + utf::string( (const char*)glGetString (GL_VERSION)));

	//clean errors (some drivers leave errors on the stack)
	while (glGetError() != GL_NO_ERROR);

	setInterfaceOrientation(Platform::singleton().getGame().getNativeOrientation());
}

Renderer::~Renderer() {
	clearLayers();
}

RenderLayer& Renderer::getLayer(RenderLayer::ID layerID) {
	//layers "always" exist
	if (layerID >= layers.size()) {
		layers.resize(layerID + 1);
	}

	return layers[layerID];
}

bool Renderer::hasLayer(RenderLayer::ID layerID) {
	return layerID < layers.size();
}

void Renderer::addRenderable(Renderable& s) {
	//get the needed layer
	RenderLayer& layer = getLayer(s.getLayer());

	DEBUG_ASSERT(layer.elements.contains(&s) == false, "This object is already registered!");

	//append at the end
	layer.elements.emplace(&s);
}

void Renderer::removeRenderable(Renderable& s) {
	if (hasLayer(s.getLayer())) {
		getLayer(s.getLayer()).elements.erase(&s);
	}

	if(lastRenderState == s) {
		lastRenderState = {};
	}
}

void Renderer::removeAllRenderables() {
	for (auto&& l : layers) {
		l.elements.clear();
	}

	lastRenderState = {};
}

void Renderer::removeViewport(const Viewport& v) {
	DEBUG_TODO;
}

void Renderer::removeAllViewports() {
	viewportList.clear();
}

void Renderer::clearLayers() {
	layers.clear();
}

void Renderer::addViewport(Viewport& v) {
	viewportList.insert(&v);
}

void Renderer::setInterfaceOrientation(Orientation o) {
	renderOrientation = o;

	static const Degrees orientations[] = {
		0.0_deg,
		180.0_deg,
		90.0_deg,
		-90.0_deg
	};

	renderRotation = orientations[(int)renderOrientation] + orientations[(int)deviceOrientation];

	//compute matrix
	mRenderRotation = glm::mat4_cast(Quaternion(Vector(0, 0, renderRotation)));
}

void Renderer::_renderElement(const RenderState& renderState) {
	auto& m = renderState.getMesh().unwrap();

	DEBUG_ASSERT( frameStarted, "Tried to render an element but the frame wasn't started" );
	DEBUG_ASSERT(m.isLoaded(), "Rendering with a mesh with no GPU data!");
	DEBUG_ASSERT(m.getVertexCount() > 0, "Rendering a mesh with no vertices");

#ifndef PUBLISH
	frameVertexCount += m.getVertexCount();
	frameTriCount += m.getPrimitiveCount();

	//each renderable is a single batch
	++frameBatchCount;
#endif // !PUBLISH

	globalUniforms.worldView = globalUniforms.view * renderState.getTransform();
	globalUniforms.worldViewProjection = globalUniforms.projection * globalUniforms.worldView;
	
	renderState.apply(globalUniforms, lastRenderState);

	static const uint32_t glModeMap[] = {
		GL_TRIANGLE_STRIP, //TriangleStrip,
		GL_TRIANGLES, //TriangleList,
		GL_LINE_STRIP, //LineStrip,
		GL_LINES, //LineList
		GL_POINTS
	};

	uint32_t mode = glModeMap[(byte)m.getTriangleMode()];

	if (!m.isIndexed()) {
		glDrawArrays(mode, 0, m.getVertexCount());
	}
	else {
		DEBUG_ASSERT(m.getIndexCount() > 0, "Rendering an indexed mesh with no indices");
		glDrawElements(mode, m.getIndexCount(), m.getIndexGLType(), nullptr); //on OpenGLES, we have max 65536 indices!!!
	}

	lastRenderState = renderState;

#ifndef DOJO_DISABLE_VAOS
	glBindVertexArray( 0 );
#endif
}

bool _cull(const RenderLayer& layer, const Viewport& viewport, const Renderable& r) {
	return layer.orthographic ? viewport.isInViewRect(r) : viewport.isContainedInFrustum(r);
}

void Renderer::_renderLayer(Viewport& viewport, const RenderLayer& layer) {
	if (!layer.elements.size() || !layer.visible) {
		return;
	}

#ifdef DOJO_WIREFRAME_AVAILABLE
	glPolygonMode(GL_FRONT_AND_BACK, layer.wireframe ? GL_LINE : GL_FILL);
#endif

	//make state changes
	if (layer.depthCheck) {
		glEnable(GL_DEPTH_TEST);

		if (layer.depthClear) {
			glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		}
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}

	//set projection state
	globalUniforms.projection = mRenderRotation * (layer.orthographic ? viewport.getOrthoProjectionTransform() : viewport.getPerspectiveProjectionTransform());

	for (auto&& r : layer.elements) {
		if (r->canBeRendered() && _cull(layer, viewport, *r)) {
			_renderElement(*r);
		}
	}
}

void Renderer::_renderViewport(Viewport& viewport) {
	if (auto rt = viewport.getRenderTarget().to_ref()) {
		rt.get().bindAsRenderTarget(true);    //TODO guess if this viewport doesn't render 3D layers to save memory?
		glFrontFace(GL_CW); //invert vertex winding when inverting the view
		globalUniforms.targetDimension = {
			(float)rt.get().getWidth(),
			(float)rt.get().getHeight()
		};
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glFrontFace(GL_CCW); //invert vertex winding when inverting the view
		globalUniforms.targetDimension = { (float)width, (float)height };
	}

	glViewport(0, 0, (GLsizei) globalUniforms.targetDimension.x, (GLsizei)globalUniforms.targetDimension.y);

	//clear the viewport
	if (viewport.getClearEnabled()) {
		glClearColor(
			viewport.getClearColor().r,
			viewport.getClearColor().g,
			viewport.getClearColor().b,
			viewport.getClearColor().a);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	globalUniforms.view = viewport.getViewTransform();
	globalUniforms.viewDirection = viewport.getWorldDirection();

	if (viewport.getVisibleLayers().empty()) { //using the default layer ordering/visibility
		for (auto&& l : layers) {
			_renderLayer(viewport, l);
		}
	}
	else { //use the custom layer ordering/visibility
		for (auto&& layer : viewport.getVisibleLayers()) {
			_renderLayer(viewport, getLayer(layer));
		}
	}
}

void Renderer::_updateRenderables(const LayerList& layers, float dt) {
	for (auto&& layer : layers) {
		for (auto&& r : layer.elements) {
			if ((r->getObject().isActive() && r->isVisible()) || r->getGraphicsAABB().isEmpty()) {
				r->update(dt);
			}
		}
	}
}

void Renderer::renderFrame(float dt) {
	DEBUG_ASSERT( !frameStarted, "Tried to start rendering but the frame was already started" );

	frameVertexCount = frameTriCount = frameBatchCount = 0;
	frameStarted = true;

	//update all the renderables
	_updateRenderables(layers, dt);

	//render all the viewports
	for (auto&& viewport : viewportList) {
		_renderViewport(*viewport);
	}

	frameStarted = false;

	submitter.get().submitFrame();
}
