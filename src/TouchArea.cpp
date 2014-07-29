#include "stdafx.h"

#include "TouchArea.h"

#include "GameState.h"
#include "Renderable.h"

using namespace Dojo;

TouchArea::TouchArea(  Listener* l, Dojo::Object* parent, const Vector& pos, const Vector& size, int layer ) :
Object( parent, pos, size ),
listener( l ),
mPressed( false ),
mTouches( 0 ),
mLayer( layer )
{
    DEBUG_ASSERT( listener, "TouchArea needs a non-null Listener" );
    
    mNeedsAABB = true;
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
    
    mNeedsAABB = true;
    getGameState()->addTouchArea( this );
}

TouchArea::~TouchArea()
{
    getGameState()->removeTouchArea( this );
}