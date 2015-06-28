#include "GameState.h"

#include "Game.h"
#include "Object.h"
#include "Sprite.h"
#include "Renderer.h"
#include "Viewport.h"
#include "Platform.h"
#include "TouchArea.h"
#include "InputSystem.h"

using namespace Dojo;

GameState::GameState(Game& parentGame) :
	Object(*this, Vector::Zero, Vector::One),
	ResourceGroup(),
	game(parentGame),
	camera(nullptr),
	timeElapsed(0) {
	gameState = this; //useful to pass a GameState around as an Object
}

GameState::~GameState() {
	clear();
}

void GameState::clear() {
	removeAllChildren();

	//flush resources
	unloadResources(false);
}

void GameState::setViewport(Viewport& v) {
	camera = &v;

	Platform::singleton().getRenderer().addViewport(v);
}

void GameState::touchAreaAtPoint(const Touch& touch) {
	Vector pointer = getViewport()->makeWorldCoordinates(touch.point);

	std::vector<TouchArea*> layer;
	int topMostLayer = INT32_MIN;

	for (auto t : mTouchAreas) {
		if (t->isActive() && t->getLayer() >= topMostLayer && t->contains2D(pointer)) {
			//new highest layer - discard lowest layers found
			if (t->getLayer() > topMostLayer)
				layer.clear();

			layer.push_back(t);

			topMostLayer = t->getLayer();
		}
	}

	//trigger all the areas overlapping in the topmost layer 
	for (auto&& l : layer)
		l->_incrementTouches(touch);
}

void GameState::addTouchArea(TouchArea* t) {
	DEBUG_ASSERT(t != nullptr, "addTouchArea: area passed was null");

	mTouchAreas.push_back(t);
}

void GameState::removeTouchArea(TouchArea* t) {
	DEBUG_ASSERT(t != nullptr, "removeTouchArea: area passed was null");

	auto elem = std::find(mTouchAreas.begin(), mTouchAreas.end(), t);
	if (elem != mTouchAreas.end())
		mTouchAreas.erase(elem);
}

void GameState::updateClickableState() {
	//clear all the touchareas
	for (auto ta : mTouchAreas)
		ta->_clearTouches();

	auto& touchList = Platform::singleton().getInput().getTouchList();

	//"touch" all the touchareas active in this frame
	for (auto&& touch : touchList)
		touchAreaAtPoint(*touch);

	///launch events
	for (auto ta : mTouchAreas)
		ta->_fireOnTouchUsingCurrentTouches();
}

void GameState::onLoop(float dt) {
	updateClickableState();

	updateChilds(dt);
}

void GameState::begin() {
	active = true;
	StateInterface::begin();
}

void GameState::end() {
	active = false;
	StateInterface::end();
}
