#include "stdafx.h"

#include "TouchArea.h"

#include "GameState.h"

using namespace Dojo;

TouchArea::TouchArea(  Listener* l, Dojo::GameState* state, const Vector& pos, const Vector& size, int layer ) :
Object( state, pos, size ),
listener( l ),
mPressed( false ),
mTouches( 0 ),
mLayer( layer )
{
    DEBUG_ASSERT( listener, "TouchArea needs a non-null Listener" );
    
    mNeedsAABB = true;
    state->addTouchArea( this );
}

TouchArea::TouchArea( Renderable* r, Listener* l ) :
Object( r->getGameState(), Vector::ZERO, r->getSize() ),
listener( l ),
mPressed( false ),
mTouches( 0 ),
mLayer( r->getLayer() )
{
    DEBUG_ASSERT( listener, "TouchArea needs a non-null Listener" );
    
    mNeedsAABB = true;
    getGameState()->addTouchArea( this );
}

TouchArea::~TouchArea()
{
    getGameState()->removeTouchArea( this );
}