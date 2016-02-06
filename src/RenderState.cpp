#include "RenderState.h"

#include "Texture.h"
#include "Mesh.h"
#include "FrameSet.h"
#include "Timer.h"
#include "Shader.h"
#include "range.h"

using namespace Dojo;

RenderState::GLBlend::GLBlend() : GLBlend(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, true) {}

static const RenderState::GLBlend modeToGLTable[] = {
	{ 0, 0, 0 }, //None
	{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD }, //alphablend
	{ GL_DST_COLOR, GL_ZERO, GL_FUNC_ADD }, //multiply
	{ GL_ONE, GL_ONE, GL_FUNC_ADD }, //add
	{ GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_FUNC_ADD }, //invert
	{ GL_ONE, GL_ONE, GL_FUNC_SUBTRACT } //subtract
};

RenderState::RenderState() :
	cullMode(CullMode::Back),
	mTransform(0) //make sure it's invalid at first
{

}

RenderState::~RenderState() {

}

void RenderState::_updateTransparency() {
	mTransparency = false;
	for(auto i : range(maxTextureSlots)) {
		if(auto t = textures[i].cast()) {
			mTransparency |= t.get().hasTransparency();
		}
	}

	if(mesh.is_some()) {
		mTransparency |= mesh.unwrap().hasVertexTransparency();
	}
}

void RenderState::setMesh(Mesh& m) {
	mesh = m;
	_updateTransparency();
}

void RenderState::setTexture(optional_ref<Texture> tex, byte ID /*= 0*/) {
	DEBUG_ASSERT(ID < textures.size(), "An ID passed to setTexture must be smaller than DOJO_MAX_TEXTURE_UNITS");

	textures[ID] = tex;

	//find the new highest slot in use
	for (maxTextureSlots = byte(textures.size()-1); maxTextureSlots >= 0; --maxTextureSlots) {
		if(textures[maxTextureSlots].is_some()) {
			break;
		}
	}
	++maxTextureSlots;

	_updateTransparency();
}

bool RenderState::isBlendingEnabled() const {
	return blending.isAuto() ? (mTransparency || color.a < 1.f) : (blending.func > 0);
}

void RenderState::setBlending(BlendingMode mode) {
	blending = modeToGLTable[(int)mode];
}

void RenderState::setShader(Shader& shader) {
	mShader = shader;
}

optional_ref<Texture> RenderState::getTexture(int ID /*= 0*/) const {
	DEBUG_ASSERT(ID >= 0, "Can't retrieve a negative textureUnit");
	DEBUG_ASSERT(ID < DOJO_MAX_TEXTURES, "An ID passed to getTextureUnit must be smaller than DOJO_MAX_TEXTURE_UNITS");

	return textures[ID];
}

void RenderState::apply(const GlobalUniformData& currentState, optional_ref<const RenderState> lastState) const {
	auto prev = lastState.to_raw_ptr();

	bool rebindFormat = false;
	if( !prev || prev->mesh != mesh ) {
		//when the mesh changes, the uniforms have to be rebound too
		rebindFormat = true;
		mesh.unwrap().bind();
	}

	if(!prev || prev->mShader != mShader) {
		rebindFormat = true;
		mShader.unwrap().bind();
	}

	if(rebindFormat) {
		mesh.unwrap().bindVertexFormat(mShader.unwrap());
	}

	mShader.unwrap().loadUniforms(currentState, self);

	for (auto i : range(maxTextureSlots)) {
		//select current slot and load it, others can remain bound to old stuff with shaders
		if (auto t = textures[i].cast()) {
			if (!prev || textures[i] != prev->textures[i]) {
				t.get().bind(i);
			}
		}
	}

	bool useBlending = isBlendingEnabled();
	if (!prev || prev->isBlendingEnabled() != useBlending) {
		if (useBlending) {
			glEnable(GL_BLEND);
		}
		else {
			glDisable(GL_BLEND);
		}
	}

	if (!prev || prev->blending.src != blending.src || prev->blending.dest != blending.dest) {
		glBlendFunc(blending.src, blending.dest);
	}

	if (!prev || prev->blending.func != blending.func) {
		glBlendEquation(blending.func);
	}

	if (!prev || prev->cullMode != cullMode) {
		switch (cullMode) {
		case CullMode::None:
			glDisable(GL_CULL_FACE);
			break;

		case CullMode::Back:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			break;

		case CullMode::Front:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			break;
		}
	}
}
