#include "stdafx.h"

#include "Viewport.h"

#include "Renderer.h"
#include "Platform.h"
#include "Game.h"
#include "AnimatedQuad.h"
#include "Sprite.h"
#include "GameState.h"
#include "SoundManager.h"
#include "Texture.h"
#include "RenderLayer.h"

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
clearColor( clear ),
VFOV( 0 ),
zNear( 0 ),
zFar( 1000 ),
mRT( nullptr )
{
	//default size is window size
	targetSize.x = (float)Platform::singleton().getWindowWidth();
	targetSize.y = (float)Platform::singleton().getWindowHeight();
	
	if( _VFOV > 0 )
		enableFrustum( _VFOV, _zNear, _zFar );
}


Viewport::~Viewport()
{

}

Vector Viewport::makeWorldCoordinates(int x, int y)
{
	return Vector(
		getWorldMin().x + ((float)x / Platform::singleton().getWindowWidth()) * size.x,
		getWorldMax().y - ((float)y / Platform::singleton().getWindowHeight()) * size.y);
}

bool Viewport::isVisible(Renderable& s) {
	return s.isVisible() && isInViewRect(s);
}

void Viewport::addFader( int layer )
{
	DEBUG_ASSERT( !faderObject, "A fade overlay object already exists" );

	//create the fader object			
	auto r = make_unique<Renderable>( getGameState(), Vector( 0,0, -1), "texturedQuad" );
	r->color = Color::NIL;

	r->scale.x = size.x;
	r->scale.y = size.y;

	r->setVisible( false );

	faderObject = &addChild( std::move(r), layer );
}

void Viewport::setRenderTarget(Texture *target)
{
    mRT = target;
    
    setTargetSize( target ?
                  Vector( (float)target->getWidth(), (float)target->getHeight() ) :
                  Vector( (float)Platform::singleton().getWindowWidth(), (float)Platform::singleton().getWindowHeight() ) );
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

	//compute local frustum vertices
	//order is - top left, bottom left, bottom right, top right, z is negative because OpenGL is right-handed
	farPlaneSide.z = -zFar;
	farPlaneSide.y = zFar * tan( Math::toRadian( VFOV*0.5f ) );
	farPlaneSide.x = ((float)targetSize.x/(float)targetSize.y) * farPlaneSide.y;

	localFrustumVertices[0] = Vector( farPlaneSide.x, farPlaneSide.y, farPlaneSide.z );
	localFrustumVertices[1] = Vector( -farPlaneSide.x, farPlaneSide.y, farPlaneSide.z );
	localFrustumVertices[2] = Vector( -farPlaneSide.x, -farPlaneSide.y, farPlaneSide.z );
	localFrustumVertices[3] = Vector( farPlaneSide.x, -farPlaneSide.y, farPlaneSide.z );

	frustumDirty = true;
}

void Viewport::_updateFrustum()
{
	if (frustumDirty) {
		//compute frustum projection
		mFrustumTransform = mPerspectiveEyeTransform * glm::perspective(
			VFOV, targetSize.x / targetSize.y,
			zNear,
			zFar);

		if (getRenderTarget()) //flip the projections to flip the image
			mOrthoTransform[1][1] *= -1;
		
		for (int i = 0; i < 4; ++i)
			worldFrustumVertices[i] = getWorldPosition(localFrustumVertices[i]);

		Vector worldPosition = getWorldPosition();

		for (int i = 0; i < 4; ++i)
		{
			int i2 = (i + 1) % 4;

			worldFrustumPlanes[i].setup(worldPosition, worldFrustumVertices[i2], worldFrustumVertices[i]);
		}
		//far plane
		worldFrustumPlanes[4].setup(worldFrustumVertices[2], worldFrustumVertices[1], worldFrustumVertices[0]);

		frustumDirty = false;
	}
}

void Viewport::_updateTransforms()
{
	updateWorldTransform();

	if (lastWorldTransform != mWorldTransform) {
		mViewTransform = glm::inverse(mWorldTransform);

		//DEBUG_ASSERT( Matrix(1) == (mViewTransform * mWorldTransform ) );

		//TODO only compute projections if the params change
		//compute ortho projection
		{
			mOrthoTransform = glm::ortho(-getHalfSize().x,
				getHalfSize().x,
				-getHalfSize().y,
				getHalfSize().y,
				0.f,  //zNear has to be 0 in ortho because in 2D mode objects with default z (0) need to be seen!
				zFar);

			if (getRenderTarget()) //flip the projections to flip the image
				mOrthoTransform[1][1] *= -1;
		}

		frustumDirty = true;

		lastWorldTransform = mWorldTransform;
	}

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
	float xf = 1.f - (screenSpacePos.x / (float) Platform::singleton().getScreenWidth());
	float yf = screenSpacePos.y / (float) Platform::singleton().getScreenHeight();
	
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

void Viewport::setVisibleLayers(int min, int max) {
	mLayerList.clear();

	for (int i = min; i < max; ++i)
		mLayerList.push_back(i);
}

bool Viewport::isContainedInFrustum(const Renderable& r) const
{
	Vector halfSize = (r.getWorldMax() - r.getWorldMin()) * 0.5f;
	Vector worldPos = r.getWorldPosition();

	//for each plane, check where the AABB is placed
	for (int i = 0; i < 4; ++i)
	{
		if (worldFrustumPlanes[i].getSide(worldPos, halfSize) < 0)
		{
			//DEBUG_MESSAGE( "CULED!!!!    " << i );
			return false;
		}
	}

	return true;
}

bool Viewport::isInViewRect(const Renderable& r) const {
	return Math::AABBsCollide2D(r.getWorldMax(), r.getWorldMin(), getWorldMax(), getWorldMin());
}

void Viewport::onAction(float dt)
{
	Object::onAction(dt);

	_updateTransforms();

	//if it has no RT, it's the main viewport - use it to set the sound listener
	if (!mRT)
		Platform::singleton().getSoundManager().setListenerTransform( getWorldTransform() );
}

