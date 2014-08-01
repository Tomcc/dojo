#include "stdafx.h"

#include "Viewport.h"

#include "Render.h"
#include "Platform.h"
#include "Game.h"
#include "AnimatedQuad.h"
#include "Sprite.h"
#include "GameState.h"
#include "SoundManager.h"
#include "Texture.h"

using namespace Dojo;

Viewport::Viewport( 
		 Object* parent, 
		 const Vector& pos,
		 const Vector& size, 
		 const Color& clear, 
		 float _VFOV, 
		 float _zNear,
		 float _zFar ) :
Object( parent, pos, size ),
cullingEnabled( true ),
fadeObject( NULL ),
clearColor( clear ),
frustumCullingEnabled( false ),
VFOV( 0 ),
zNear( 0 ),
zFar( 1000 ),
mRT( nullptr )
{
    mNeedsAABB = true;

	//default size is window size
	targetSize.x = (float)Platform::getSingleton()->getWindowWidth();
	targetSize.y = (float)Platform::getSingleton()->getWindowHeight();
	
	if( _VFOV > 0 )
		enableFrustum( _VFOV, _zNear, _zFar );
}


Viewport::~Viewport()
{

}

Vector Viewport::makeWorldCoordinates(int x, int y)
{
	return Vector(
		getWorldMin().x + ((float)x / Platform::getSingleton()->getWindowWidth()) * size.x,
		getWorldMax().y - ((float)y / Platform::getSingleton()->getWindowHeight()) * size.y);
}

bool Viewport::isSeeing(Renderable* s) {
	DEBUG_ASSERT(s, "isSeeing: null renderable passed");

	return cullingEnabled && s->isVisible() && touches(s);
}

void Viewport::addFader( int layer )
{
	DEBUG_ASSERT( !fadeObject, "A fade overlay object already exists" );

	//create the fader object			
	fadeObject = new Renderable( getGameState(), Vector( 0,0, -1), "texturedQuad" );
	fadeObject->color = Color::NIL;

	fadeObject->scale.x = size.x;
	fadeObject->scale.y = size.y;

	fadeObject->setVisible( false );

	addChild( fadeObject, layer );
}

void Viewport::setRenderTarget(Texture *target)
{
    mRT = target;
    
    setTargetSize( target ?
                  Vector( (float)target->getWidth(), (float)target->getHeight() ) :
                  Vector( (float)Platform::getSingleton()->getWindowWidth(), (float)Platform::getSingleton()->getWindowHeight() ) );
}

void Viewport::lookAt(  const Vector& worldPos )
{
	setRotation( glm::quat_cast( glm::lookAt( getWorldPosition(), worldPos, Vector::NEGATIVE_UNIT_Y ) ) ); //HACK why negative does work? Up is +Y
}

void Viewport::enableFrustum( float _VFOV, float _zNear, float _zFar )
{
	//assert some reasonable values
	DEBUG_ASSERT( _zNear > 0, "Z near value cannot be negative or 0" );
	DEBUG_ASSERT( _zNear < _zFar, "Z far cannot be less than Z near" );
	DEBUG_ASSERT( _VFOV > 0 && _VFOV < 180, "Vertical FOV has to be in the range ]0..180[" );

	VFOV = _VFOV;
	zNear = _zNear;
	zFar = _zFar;

	frustumCullingEnabled = true;

	//compute local frustum vertices
	//order is - top left, bottom left, bottom right, top right, z is negative because OpenGL is right-handed
	farPlaneSide.z = -zFar;
	farPlaneSide.y = zFar * tan( Math::toRadian( VFOV*0.5f ) );
	farPlaneSide.x = ((float)targetSize.x/(float)targetSize.y) * farPlaneSide.y;

	localFrustumVertices[0] = Vector( farPlaneSide.x, farPlaneSide.y, farPlaneSide.z );
	localFrustumVertices[1] = Vector( -farPlaneSide.x, farPlaneSide.y, farPlaneSide.z );
	localFrustumVertices[2] = Vector( -farPlaneSide.x, -farPlaneSide.y, farPlaneSide.z );
	localFrustumVertices[3] = Vector( farPlaneSide.x, -farPlaneSide.y, farPlaneSide.z );
}

void Viewport::_updateFrustum()
{
	for( int i = 0; i < 4; ++i )
		worldFrustumVertices[i] = getWorldPosition( localFrustumVertices[i] );
    
    Vector worldPosition = getWorldPosition();

	for( int i = 0; i < 4; ++i )
	{
		int i2 = (i+1)%4;

		worldFrustumPlanes[i].setup( worldPosition, worldFrustumVertices[i2], worldFrustumVertices[i] );
	}
	//far plane
	worldFrustumPlanes[4].setup( worldFrustumVertices[2], worldFrustumVertices[1], worldFrustumVertices[0] );
}

void Viewport::_updateTransforms()
{
    //compute view, that is the inverse of our world matrix        
    /*Vector worldPos = getWorldPosition();
    
    glm::vec3 t( -worldPos.x, -worldPos.y, -worldPos.z );

	mViewTransform = glm::translate( Matrix(1), t );
	mViewTransform = glm::scale( mViewTransform, Vector( 1.f/scale.x, 1.f/scale.y, 1.f/scale.z ) );
	mViewTransform *= glm::mat4_cast( glm::inverse( rotation ) );*/

	updateWorldTransform();

	//TODO use something faster than glm::inverse
	mViewTransform = glm::inverse( mWorldTransform );

	//DEBUG_ASSERT( Matrix(1) == (mViewTransform * mWorldTransform ) );
	
	//TODO only compute projections if the params change
    //compute ortho projection
    mOrthoTransform = glm::ortho(-getHalfSize().x, 
                              getHalfSize().x,
                              -getHalfSize().y,
                              getHalfSize().y,
                              0.f,  //zNear has to be 0 in ortho because in 2D mode objects with default z (0) need to be seen!
                              zFar );
       
    //compute frustum projection
    mFrustumTransform = mPerspectiveEyeTransform * glm::perspective(
                                         VFOV, targetSize.x / targetSize.y, 
                                         zNear, 
                                         zFar );

	if( getRenderTarget() ) //flip the projections to flip the image
	{
		mOrthoTransform[1][1] *= -1;
		mFrustumTransform[1][1] *= -1;
	}
}

bool Viewport::isContainedInFrustum( Renderable* r )
{
	//HACK
	return true;

	DEBUG_ASSERT( r, "The passed renderable was null" );

	Vector halfSize = (r->getWorldMax() - r->getWorldMin()) * 0.5f;
	Vector worldPos = r->getWorldPosition();

	//for each plane, check where the AABB is placed
	for( int i = 0; i < 4; ++i )
	{
		if( worldFrustumPlanes[i].getSide( worldPos, halfSize ) < 0 )
		{
			//DEBUG_MESSAGE( "CULED!!!!    " << i );
			return false;
		}
	}

	return true;
}

Vector Viewport::getScreenPosition( const Vector& pos )
{
	//get local position
	Vector local = getLocalPosition( pos );

	//project local on the local far plane
	float f = (zFar/local.z);
	local.x *= f;
	local.y *= f;

	//bring in screen space
	local.x = -(local.x / farPlaneSide.x) * halfSize.x;
	local.y = (local.y / farPlaneSide.y) * halfSize.y;

	return local;
}

Vector Viewport::getRayDirection( const Vector& screenSpacePos )
{
	//frustum[0]: top left
	//frustum[1]: bottom left
	//frustum[2]: bottom right
	//frustum[3]: top right
	float xf = 1.f - (screenSpacePos.x / (float) Platform::getSingleton()->getScreenWidth());
	float yf = screenSpacePos.y / (float) Platform::getSingleton()->getScreenHeight();
	
	//find points on each side of the frustum
	Vector a = worldFrustumVertices[0].lerpTo( xf, worldFrustumVertices[1] );
	Vector b = worldFrustumVertices[3].lerpTo( xf, worldFrustumVertices[2] );
		
	//now we can find the final far plane projection and the ray direction
	a = a.lerpTo( yf, b );
	
	return (a - getWorldPosition()).normalized();
}

void Viewport::makeScreenSize( Vector& dest, Texture* tex )
{
	makeScreenSize( dest, tex->getWidth(), tex->getHeight() );
}

void Viewport::setVisibleLayers( const LayerList& layers )
{
	mLayerList = layers;
}

void Viewport::onAction(float dt)
{
	Object::onAction(dt);

	_updateTransforms();

	//do not call if not explicitly required
	if (frustumCullingEnabled)
		_updateFrustum();

	//if it has no RT, it's the main viewport - use it to set the sound listener
	if (!mRT)
		Platform::getSingleton()->getSoundManager()->setListenerTransform( getWorldTransform() );
}

