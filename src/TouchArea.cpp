#include "TouchArea.h"

#include "GameState.h"
#include "Renderable.h"

using namespace Dojo;

TouchArea::TouchArea(Listener& l, Object& parent, const Vector& pos, const Vector& size, int layer) :
	Object(parent, pos, size),
	listener(l),
	mPressed(false),
	mTouches(0),
	mLayer(layer) {

	getGameState().addTouchArea(self);
}

TouchArea::TouchArea(Renderable& r, Listener& l) :
	Object(r.getGameState(), Vector::Zero, r.getObject().getSize()), //TODO this should use the renderable's screensize instead
	listener(l),
	mPressed(false),
	mTouches(0),
	mLayer(r.getLayer()) {

	getGameState().addTouchArea(self);
}

TouchArea::~TouchArea() {
	getGameState().removeTouchArea(self);
}

void TouchArea::_fireOnTouchUsingCurrentTouches() {
	//set the state to clicked if there's at least one touch
	bool active = mTouches.size() > 0;

	if (mPressed != active) { ///fire the event on state change
		mPressed = active;

		//if all the touches just began in this area, the user tapped
		if (mPressed) {
			bool tapped = true;

			for (auto&& t : mTouches) {
				if (not t.firstFrame) {
					tapped = false;
					break;
				}
			}

			if (tapped) {
				listener.unwrap().onTouchAreaTapped(self);
			}
		}

		if (mPressed) {
			listener.unwrap().onTouchAreaPressed(self);
		}
		else {
			listener.unwrap().onTouchAreaReleased(self);
		}
	}
}

void TouchArea::_notifyLayer(int l) {
	mLayer = l;
}

void TouchArea::_clearTouches() {
	mTouches.clear();
}

void TouchArea::_incrementTouches(const Touch& touch) {
	mTouches.emplace_back(touch);
}

void TouchArea::setListener(Listener& l) {
	listener = l;
}

int TouchArea::getLayer() const {
	return top ? INT_MAX : mLayer;
}

const TouchArea::TouchList& TouchArea::getTouchList() const {
	return mTouches;
}

bool TouchArea::isPressed() const {
	return mPressed;
}

bool TouchArea::contains2D(const Vector& p) const {
	return
		p.x <= worldBB.max.x and
		p.x >= worldBB.min.x and
		p.y <= worldBB.max.y and
		p.y >= worldBB.min.y;
}

void TouchArea::onAction(float dt) {
	Object::onAction(dt);

	worldBB = transformAABB({ -getHalfSize(), getHalfSize() });
}
