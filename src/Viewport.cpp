#include "stdafx.h"

#include "Viewport.h"

#include "Render.h"
#include "Platform.h"
#include "Game.h"
#include "AnimatedQuad.h"
#include "Sprite.h"
#include "GameState.h"

using namespace Dojo;

Viewport::Viewport( 
		 GameState* level, 
		 const Vector& pos,
		 const Vector& size, 
		 const Color& clear, 
		 float _VFOV, 
		 float _zNear,
		 float _zFar ) :
Object( level, pos, size ),
cullingEnabled( true ),
fadeObject( NULL ),
background( NULL ),
clearColor( clear ),
frustumCullingEnabled( false ),
VFOV( 0 ),
zNear( 0 ),
zFar( 1000 )
{
    mNeedsAABB = true;
    
	Render* render = Platform::getSingleton()->getRender();
	
	nativeToScreenRatio = render->getNativeToScreenRatio();
	
	//default size is window size
	targetSize.x = (float)Platform::getSingleton()->getWindowWidth();
	targetSize.y = (float)Platform::getSingleton()->getWindowHeight();
	
	if( _VFOV > 0 )
		enableFrustum( _VFOV, _zNear, _zFar );
}	

void Viewport::addFader( int layer )
{
	DEBUG_ASSERT( !fadeObject );

	//create the fader object			
	fadeObject = new Renderable( getGameState(), Vector( 0,0, -1), "texturedQuad" );
	fadeObject->color = Color::NIL;

	fadeObject->scale.x = size.x;
	fadeObject->scale.y = size.y;

	fadeObject->setVisible( false );

	addChild( fadeObject, layer );
}

void Viewport::lookAt(  const Vector& worldPos )
{
	setRotation( glm::quat_cast( glm::lookAt( getWorldPosition(), worldPos, Vector::NEGATIVE_UNIT_Y ) ) ); //HACK why negative does work? Up is +Y
}

void Viewport::setBackgroundSprite( const String& name, float frameTime )
{			
	DEBUG_ASSERT( name.size() );
	
	if( background )
		destroyChild( background );
	
	background = new Sprite( gameState, Vector::ZERO, name, frameTime );
	background->setBlendingEnabled( false );
	background->setVisible( true );
	
	//force the proportions to fill screen
	background->_updateScreenSize();
	
	//the background image must not be stretched on different aspect ratios
	//so we just pick the pixel size for the horizontal			
	background->pixelScale.x = (float)background->getTexture(0)->getWidth() / (float)targetSize.x;
	background->pixelScale.y = background->pixelScale.x;	
	
	addChild( background );
}

void Viewport::enableFrustum( float _VFOV, float _zNear, float _zFar )
{
	//assert some reasonable values
	DEBUG_ASSERT( _zNear > 0 );
	DEBUG_ASSERT( _zNear < _zFar );
	DEBUG_ASSERT( _VFOV > 0 && _VFOV < 180 );

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

	//TODO use something faster than glm::inverse
	mViewTransform = glm::inverse( mWorldTransform );

	//DEBUG_ASSERT( Matrix(1) == (mViewTransform * mWorldTransform ) );
	
	//TODO only compute projections if the params change
    //compute ortho projection
    mOrthoTransform = glm::ortho(-getHalfSize().x, 
                              getHalfSize().x,
                              -getHalfSize().y,
                              getHalfSize().y,
                                 zNear,
                                 zFar );
       
    //compute frustum projection
    mFrustumTransform = glm::perspective( 
                                         VFOV, targetSize.x / targetSize.y, 
                                         zNear, 
                                         zFar );
}

bool Viewport::isContainedInFrustum( Renderable* r )
{
	DEBUG_ASSERT( r );

	Vector halfSize = (r->getWorldMax() - r->getWorldMin()) * 0.5f;
	Vector worldPos = r->getWorldPosition();

	//for each plane, check where the AABB is placed
	for( uint i = 0; i < 4; ++i )
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
	
	//find points on each side of the frustum
	Vector a = worldFrustumVertices[1].lerp( screenSpacePos.y, worldFrustumVertices[0] );
	Vector b = worldFrustumVertices[2].lerp( screenSpacePos.y, worldFrustumVertices[3] );
		
	//now we can find the final far plane projection and the ray direction
	a = b.lerp( screenSpacePos.x, a );
	
	return (a - getWorldPosition()).normalized();
}

void Viewport::makeScreenSize( Vector& dest, Texture* tex )
{
	makeScreenSize( dest, tex->getWidth(), tex->getHeight() );
}

