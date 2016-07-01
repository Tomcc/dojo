#include "Renderable.h"

#include "Game.h"
#include "Viewport.h"
#include "Mesh.h"
#include "GameState.h"
#include "Object.h"
#include "Platform.h"
#include "Renderer.h"

using namespace Dojo;

Renderable::Renderable(Object& parent, RenderLayer::ID layer) :
	Component(parent),
	layer(layer) {
	color = Color::White;
}

Renderable::Renderable(Object& parent, RenderLayer::ID layer, Mesh& m, Shader& shader) :
	Renderable(parent, layer) {
	setMesh(m);
	mShader = shader;

	DEBUG_ASSERT(mesh.unwrap().supportsShader(mShader.unwrap()), "cannot use this mesh with this shader");
}

Renderable::Renderable(Object& parent, RenderLayer::ID layer, utf::string_view meshName, utf::string_view shaderName) :
	Renderable(
		parent,
		layer, 
		parent.getGameState().getMesh(meshName).unwrap(), 
		parent.getGameState().getShader(shaderName).unwrap()) {

	DEBUG_ASSERT(mesh.unwrap().supportsShader(mShader.unwrap()), "cannot use this mesh with this shader");
}

Renderable::~Renderable() {

}

void Renderable::startFade(const Color& start, const Color& end, float duration) {
	DEBUG_ASSERT(duration > 0, "The duration of a fade must be greater than 0");

	fadeStartColor = start;
	fadeEndColor = end;

	color = start;

	currentFadeTime = 0;

	fadeEndTime = duration;

	fading = true;

	setVisible(true);
}

void Renderable::startFade(float startAlpha, float endAlpha, float duration) {
	color.a = startAlpha;

	Color end = color;
	end.a = endAlpha;

	startFade(color, end, duration);
}

void Renderable::update(float dt) {
	if (auto m = mesh.to_ref()) {
		auto trans = glm::scale(object.getWorldTransform(), scale);

		advanceFade(dt);

		if (trans != mTransform) {
			AABB bounds = m.get().getBounds();
			bounds.max = Vector::mul(bounds.max, scale);
			bounds.min = Vector::mul(bounds.min, scale);
			mWorldBB = object.transformAABB(bounds);
			mTransform = trans;
		}
	}
}

bool Renderable::canBeRendered() const {
	if (auto m = mesh.to_ref()) {
		return isVisible() and m.get().isLoaded() and m.get().getVertexCount() > 2;
	}
	else {
		return false;
	}
}

void Renderable::stopFade() {
	fading = false;
}

void Renderable::advanceFade(float dt) {
	if (fading) { //fade is scheduled
		float fade = currentFadeTime / fadeEndTime;
		float invf = 1.f - fade;

		color.r = fadeEndColor.r * fade + invf * fadeStartColor.r;
		color.g = fadeEndColor.g * fade + invf * fadeStartColor.g;
		color.b = fadeEndColor.b * fade + invf * fadeStartColor.b;
		color.a = fadeEndColor.a * fade + invf * fadeStartColor.a;

		if (currentFadeTime > fadeEndTime) {
			fading = false;

			if (fadeEndColor.a == 0) {
				setVisible(false);
			}
		}

		currentFadeTime += dt;
	}
}

GameState& Renderable::getGameState() const {
	return getObject().getGameState();
}

void Renderable::onAttach() {
	Platform::singleton().getRenderer().addRenderable(self);
}

void Renderable::onDetach() {
	Platform::singleton().getRenderer().removeRenderable(self);
}