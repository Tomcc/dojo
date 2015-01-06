#include "stdafx.h"

#include "TouchArea.h"

#include "GameState.h"
#include "Renderable.h"

using namespace Dojo;

TouchArea::TouchArea(  Listener* l, Object* parent, const Vector& pos, const Vector& size, int layer ) :
Object( parent, pos, size ),
listener( l ),
mPressed( false ),
mTouches( 0 ),
mLayer( layer )
{
    DEBUG_ASSERT( listener, "TouchArea needs a non-null Listener" );
    
    getGameState()->addTouchArea( this );
}

TouchArea::TouchArea( Renderable* r, Listener* l ) :
Object( r->getGameState(), Vector::ZERO, r->getSize() ),
listener( l ),
mPressed( false ),
mTouches( 0 ),
mLayer( r->getLayer() )
{
    DEBUG_ASSERT( listener, "TouchArea needs a non-null Listener" );
    
    getGameState()->addTouchArea( this );
}

TouchArea::~TouchArea()
{
    getGameState()->removeTouchArea( this );
}

void TouchArea::_fireOnTouchUsingCurrentTouches() {
	//set the state to clicked if there's at least one touch
	bool active = mTouches.size() > 0;

	if (mPressed != active) ///fire the event on state change
	{
		mPressed = active;

		//if all the touches just began in this area, the user tapped
		if (mPressed)
		{
			bool tapped = true;
			for (auto& t : mTouches)
			{
				if (!t.firstFrame)
				{
					tapped = false;
					break;
				}
			}
			if (tapped)		listener->onTouchAreaTapped(*this);
		}

		if (mPressed)      listener->onTouchAreaPressed(*this);
		else                listener->onTouchAreaReleased(*this);
	}
}

void TouchArea::_notifyLayer(int l) {
	mLayer = l;
}

void TouchArea::_clearTouches() {
	mTouches.clear();
}

void TouchArea::_incrementTouches(const Touch& touch) {
	mTouches.push_back(touch);
}

void TouchArea::setListener(Listener* l) {
	DEBUG_ASSERT(l, "cannot set a null listener");

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
