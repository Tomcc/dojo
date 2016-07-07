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

#include <glad/glad.h>

using namespace Dojo;

GLuint gDefaultVAO = 0;


const char* _errorToString(GLenum errorType) {
	switch (errorType)
	{
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	default:
		return "UNKNOWN ERROR";
	}
};

const char* _errorTypeToString(GLenum errorType) {
	switch (errorType) {
	case GL_DEBUG_TYPE_ERROR:
		return "GL_DEBUG_TYPE_ERROR";
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		return "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR";
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		return "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR";
	case GL_DEBUG_TYPE_PORTABILITY:
		return "GL_DEBUG_TYPE_PORTABILITY";
	case GL_DEBUG_TYPE_PERFORMANCE:
		return "GL_DEBUG_TYPE_PERFORMANCE";
	case GL_DEBUG_TYPE_OTHER:
		return "GL_DEBUG_TYPE_POP_GROUP";
	case GL_DEBUG_TYPE_MARKER:
		return "GL_DEBUG_TYPE_MARKER";
	case GL_DEBUG_TYPE_PUSH_GROUP:
		return "GL_DEBUG_TYPE_PUSH_GROUP";
	case GL_DEBUG_TYPE_POP_GROUP:
		return "GL_DEBUG_TYPE_POP_GROUP";
	default:
		FAIL("Add this error to this list pls, it's new");
	}
}

bool isGLErrorFatal(GLenum errorType) {
	switch (errorType) {
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
	case GL_DEBUG_TYPE_ERROR:
	case GL_DEBUG_TYPE_PORTABILITY:
		return true;
	default:
		return false;
	}
}

bool isGLErrorIgnored(GLenum errorType, GLenum severity) {
	switch (errorType) {
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
	case GL_DEBUG_TYPE_ERROR:
	case GL_DEBUG_TYPE_PORTABILITY:
		return false;			
	case GL_DEBUG_TYPE_PERFORMANCE:
	case GL_DEBUG_TYPE_MARKER:
		return severity == GL_DEBUG_SEVERITY_NOTIFICATION;
	default:
		return true;
	}
}


bool logInfoGLMessages = false;

void APIENTRY GL_DEBUG_CALLBACK(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message, 
	const GLvoid *userParam) {

	if (isGLErrorIgnored(type, severity)) {
		if (logInfoGLMessages) {
			Dojo::gp_log->append( utf::string("GL info: ") +  message + "\n");
		}
		return;
	}

	Dojo::gp_log->append(utf::string("OpenGL Error: ") + _errorTypeToString(type) + "\n" + "Message: " + message);

	if (isGLErrorFatal(type)) {
		FAIL("OpenGL fatal error!");
	}
}

Dojo::Renderer::Renderer(RenderSurface backbuffer, Orientation renderOrientation) :
	frameStarted(false),
	valid(true),
	renderOrientation(DO_LANDSCAPE_RIGHT),
	deviceOrientation(renderOrientation),
	mBackBuffer(backbuffer),
	frameVertexCount(0),
	frameTriCount(0),
	frameBatchCount(0),
	submitter(Platform::singleton()) {
	DEBUG_MESSAGE("Creating OpenGL context...");
	DEBUG_MESSAGE("querying GL info... ");
	DEBUG_MESSAGE("vendor: " + utf::string((const char*)glGetString(GL_VENDOR)));
	DEBUG_MESSAGE("renderer: " + utf::string((const char*)glGetString(GL_RENDERER)));
	DEBUG_MESSAGE("version: OpenGL " + utf::string((const char*)glGetString(GL_VERSION)));

	setInterfaceOrientation(Platform::singleton().getGame().getNativeOrientation());

	//HACK GL core doesn't work without a VAO bound... but ain't nobody got time fo' dat
	glGenVertexArrays(1, &gDefaultVAO);
	glBindVertexArray(gDefaultVAO);

#ifdef PUBLISH
	bool shouldLog = false;
#else
	bool shouldLog = true;
#endif

	//let the user decide if they want the logging or not, default to the above
	if (GLAD_GL_KHR_debug and Platform::singleton().getUserConfiguration().getBool("enable_GL_log", shouldLog)) {
#ifndef PUBLISH
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#else
		glEnable(GL_DEBUG_OUTPUT);
#endif

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

		//decide if to also log normally ignored messages
		logInfoGLMessages = Platform::singleton().getUserConfiguration().getBool("verbose_GL_log");

		glDebugMessageCallback(&GL_DEBUG_CALLBACK, nullptr);
	}
}

Renderer::~Renderer() {
	clearLayers();

	if(gDefaultVAO) {
		glDeleteVertexArrays(1, &gDefaultVAO);
		gDefaultVAO = 0;
	}
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
	DEBUG_ASSERT_MAIN_THREAD;

	//get the needed layer
	RenderLayer& layer = getLayer(s.getLayerID());

	DEBUG_ASSERT(layer.elements.contains(&s) == false, "This object is already registered!");

	//append at the end
	layer.elements.emplace(&s);
	layer.elementsChangedThisFrame |= true;
}

void Renderer::removeRenderable(Renderable& s) {
	DEBUG_ASSERT_MAIN_THREAD;

	if (hasLayer(s.getLayerID())) {
		auto& layer = getLayer(s.getLayerID());
		layer.elements.erase(&s);
		layer.elementsChangedThisFrame |= true;
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
	auto elem = std::find(viewportList.begin(), viewportList.end(), &v);
	DEBUG_ASSERT(elem != viewportList.end(), "Viewport not found");
	viewportList.erase(elem);
}

void Renderer::removeAllViewports() {
	viewportList.clear();
}

void Renderer::clearLayers() {
	layers.clear();
}

void Renderer::addViewport(Viewport& v, int index /*= -1*/) {
	if (index < 0) { 
		viewportList.push_back(&v);
	}
	else {
		DEBUG_ASSERT(index <= (int)viewportList.size(), "Invalid index");
		viewportList.insert(viewportList.begin() + index, 1, &v);
	}
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

void Dojo::Renderer::_renderElement(const RenderLayer& layer, const RenderState& renderState) {
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

	globalUniforms.world = renderState.getTransform();
	globalUniforms.world[3][2] += layer.zOffset;

	globalUniforms.worldView = globalUniforms.view * globalUniforms.world;
	globalUniforms.worldViewProjection = globalUniforms.projection * globalUniforms.worldView;
	
	renderState.apply(globalUniforms, lastRenderState);

	static const uint32_t glModeMap[] = {
		GL_TRIANGLE_STRIP, //TriangleStrip,
		GL_TRIANGLES, //TriangleList,
		GL_LINE_STRIP, //LineStrip,
		GL_LINES, //LineList
		GL_POINTS
	};

	uint32_t mode = glModeMap[(uint8_t)m.getTriangleMode()];

	if (m.isIndexed()) {
		glDrawElements(mode, m.getIndexCount(), m.getIndexGLType(), nullptr);
	}
	else {
		glDrawArrays(mode, 0, m.getVertexCount());
	}

	lastRenderState = renderState;
}

bool _cull(const RenderLayer& layer, const Viewport& viewport, const Renderable& r) {
	return layer.orthographic ? viewport.isInViewRect(r) : viewport.isContainedInFrustum(r);
}

void Renderer::_renderLayer(Viewport& viewport, const RenderLayer& layer) {
	if (layer.elements.empty() or not layer.visible) {
		return;
	}

	//depth TEST actually is required even just to write...
	if (layer.usesDepth()) {
		DEBUG_ASSERT(viewport.getFramebuffer().hasDepth(), "Depth won't work without an attachment");
		glEnable(GL_DEPTH_TEST);
		glDepthMask(layer.depthWrite);
		glDepthFunc(layer.depthTest ? GL_LESS : GL_ALWAYS);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}

	//set projection state
	globalUniforms.projection = mRenderRotation * (layer.orthographic ? viewport.getOrthoProjectionTransform() : viewport.getPerspectiveProjectionTransform());

	for (auto&& r : layer.elements) {
		if (r->canBeRendered() and _cull(layer, viewport, *r)) {
			_renderElement(layer, *r);
		}
	}
}

void Renderer::_renderViewport(Viewport& viewport) {
	viewport._update();

	viewport.getFramebuffer().bind();

	globalUniforms.targetDimension = {
		(float)viewport.getFramebuffer().getWidth(),
		(float)viewport.getFramebuffer().getHeight()
	};

	glViewport(0, 0, (GLsizei) globalUniforms.targetDimension.x, (GLsizei)globalUniforms.targetDimension.y);

	//clear the viewport
	GLuint clearFlags = 0;
	if (viewport.getColorClearEnabled()) {
		glClearColor(
			viewport.getClearColor().r,
			viewport.getClearColor().g,
			viewport.getClearColor().b,
			viewport.getClearColor().a);

		clearFlags |= GL_COLOR_BUFFER_BIT;
	}
	
	if(viewport.getDepthClearEnabled() and viewport.getFramebuffer().hasDepth()) {
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glClearDepthf(viewport.getClearDepth());
		clearFlags |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
	}

	if (clearFlags) {
		glClear(clearFlags);
	}

	globalUniforms.view = viewport.getViewTransform();
	globalUniforms.viewDirection = viewport.getObject().getWorldDirection();

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

	if(viewport.getInvalidatePreviousViewportsAfterFrame()) {
		//invalidate all viewports before this one
		for (auto&& v : viewportList) {
			if(v == &viewport) {
				break;
			}

			v->getFramebuffer().invalidate();
		}
	}
}

void Dojo::Renderer::_updateRenderables(LayerList& layers, float dt) {
	for (auto&& layer : layers) {
		do {
			//repeat the update on the whole layer if any element is added or removed
			//TODO this isn't risky but very inefficient, find another way!
			layer.elementsChangedThisFrame = false;
			for (auto&& r : layer.elements) {
				if ((r->getObject().isActive() and r->isVisible()) or r->getGraphicsAABB().isEmpty()) {
					r->update(dt);
					if (layer.elementsChangedThisFrame) {
						break;
					}
				}
			}
		} while (layer.elementsChangedThisFrame);
	}
}

void Renderer::renderFrame(float dt) {
	DEBUG_ASSERT(not frameStarted, "Tried to start rendering but the frame was already started" );

	frameVertexCount = frameTriCount = frameBatchCount = 0;
	frameStarted = true;

	//update all the renderables
	_updateRenderables(layers, dt);

	//render all the viewports
	for (auto&& viewport : viewportList) {
		_renderViewport(*viewport);
	}

	frameStarted = false;
}

void Renderer::endFrame() {
	submitter.get().submitFrame();
}
