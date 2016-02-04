#include "RenderState.h"

#include "Texture.h"
#include "Mesh.h"
#include "FrameSet.h"
#include "Timer.h"
#include "Shader.h"
#include "range.h"

using namespace Dojo;

RenderState::RenderState() :
	cullMode(CullMode::Back),
	blendingEnabled(true),
	srcBlend(GL_SRC_ALPHA),
	destBlend(GL_ONE_MINUS_SRC_ALPHA),
	blendFunction(GL_FUNC_ADD) {
	memset(textures, 0, sizeof(textures)); //zero all the textures
}

RenderState::~RenderState() {

}

void Dojo::RenderState::setTexture(optional_ref<Texture> tex, int ID /*= 0*/) {
	DEBUG_ASSERT(ID >= 0, "Passed a negative texture ID to setTexture()");
	DEBUG_ASSERT(ID < DOJO_MAX_TEXTURES, "An ID passed to setTexture must be smaller than DOJO_MAX_TEXTURE_UNITS");

	textures[ID] = tex;

	//find the new highest slot in use
	for (maxTextureSlot = DOJO_MAX_TEXTURES; maxTextureSlot >= 0; --maxTextureSlot) {
		if(textures[maxTextureSlot].is_some()) {
			break;
		}
	}
}

void RenderState::setBlending(BlendingMode mode) {
	struct GLBlend {
		GLenum src, dest, func;
	};

	static const GLBlend modeToGLTable[] = {
		{GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD}, //alphablend
		{GL_DST_COLOR, GL_ZERO, GL_FUNC_ADD}, //multiply
		{GL_ONE, GL_ONE, GL_FUNC_ADD}, //add
		{GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_FUNC_ADD}, //invert
		{GL_ONE, GL_ONE, GL_FUNC_SUBTRACT} //subtract
	};

	auto& blend = modeToGLTable[(int)mode];

	setBlending(blend.src, blend.dest);
	blendFunction = blend.func;
}

void RenderState::setShader(Shader& shader) {
	mShader = shader;
}

optional_ref<Texture> Dojo::RenderState::getTexture(int ID /*= 0*/) const {
	DEBUG_ASSERT(ID >= 0, "Can't retrieve a negative textureUnit");
	DEBUG_ASSERT(ID < DOJO_MAX_TEXTURES, "An ID passed to getTextureUnit must be smaller than DOJO_MAX_TEXTURE_UNITS");

	return textures[ID];
}

int RenderState::getDistance(RenderState* s) {
	DEBUG_ASSERT(s, "getDistance: The input RenderState is null");

	int dist = 0;

	DEBUG_TODO; //dunno
	//
	// 	if (s->mesh != mesh)
	// 		dist += 3;
	//
	// 	for (int i = 0; i < DOJO_MAX_TEXTURES; ++i) {
	// 		if (textures[i] != s->textures[i])
	// 			dist += 2;
	// 	}
	//
	// 	if (s->isAlphaRequired() != isAlphaRequired())
	// 		dist += 1;

	return dist;
}

void RenderState::apply(const GlobalUniformData& currentState) const {
	auto& shader = getShader().unwrap();
	shader.bind();
	shader.loadUniforms(currentState, self);

	mesh.unwrap().bind();

	for (auto i : range(DOJO_MAX_TEXTURES)) {
		if (auto t = textures[i].cast()) {
			t.get().bind(i);
		}
		else {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	if (blendingEnabled) {
		glEnable(GL_BLEND);
	}
	else {
		glDisable(GL_BLEND);
	}

	glBlendFunc(srcBlend, destBlend);
	glBlendEquation(blendFunction);

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

void RenderState::applyStateDiff(const GlobalUniformData& currentState, optional_ref<const RenderState> lastState) const {
	if (auto lastRef = lastState.cast()) {
		auto prev = lastRef.get();

		bool rebindFormat = false;
		if( prev.mesh != mesh ) {
			//when the mesh changes, the uniforms have to be rebound too
			rebindFormat = true;
			mesh.unwrap().bind();
		}

		if( prev.mShader != mShader) {
			rebindFormat = true;
			mShader.unwrap().bind();
		}

		if(rebindFormat) {
			mesh.unwrap().bindVertexFormat(mShader.unwrap());
		}

		mShader.unwrap().loadUniforms(currentState, self);

		for (auto i : range(maxTextureSlot)) {
			//select current slot and load it, others can remain bound to old stuff with shaders
			if (auto t = textures[i].cast()) {
				if (textures[i] != prev.textures[i]) {
					t.get().bind(i);
				}
			}
		}

		if (prev.blendingEnabled != blendingEnabled) {
			if (blendingEnabled) {
				glEnable(GL_BLEND);
			}
			else {
				glDisable(GL_BLEND);
			}
		}

		if (prev.srcBlend != srcBlend || prev.destBlend != destBlend) {
			glBlendFunc(srcBlend, destBlend);
		}

		if( prev.blendFunction != blendFunction) {
			glBlendEquation(blendFunction);
		}

		if (prev.cullMode != cullMode) {
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
	else {
		apply(currentState);
	}
}
