#include "stdafx.h"

#include "Viewport.h"

#include "Render.h"
#include "Platform.h"
#include "Model.h"
#include "Game.h"
#include "AnimatedQuad.h"
#include "Sprite.h"

using namespace Dojo;

Viewport::Viewport( 
		 GameState* level, 
		 const Vector& pos,
		 const Vector& size, 
		 const Color& clear, 
		 float _VFOV, 
		 float _zNear,
		 float _zFar,
		 int fadeObjectLayer ) :
Object( level, pos, size ),
cullingEnabled( true ),
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
	
	targetSize.x = (float)level->getGame()->getNativeWidth();
	targetSize.y = (float)level->getGame()->getNativeHeight();
	
	//create the fader object			
	fadeObject = new Model( level, Vector::ZERO, "texturedQuad", String::EMPTY );
	fadeObject->color = Color( 0, 0, 0, 0 );
	
	fadeObject->scale.x = size.x;
	fadeObject->scale.y = size.y;
	
	fadeObject->setVisible( false );
	
	addChild( fadeObject, fadeObjectLayer );
	
	if( _VFOV > 0 )
		enableFrustum( _VFOV, _zNear, _zFar );
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
	//order is - top left, bottom left, bottom right, top right
	farPlaneSide.z = zFar;
	farPlaneSide.y = zFar * tan( Math::toRadian( VFOV*0.5f ) );
	farPlaneSide.x = ((float)targetSize.x/(float)targetSize.y) * farPlaneSide.y;

	localFrustumVertices[0] = Vector( farPlaneSide.x, farPlaneSide.y, farPlaneSide.z );
	localFrustumVertices[1] = Vector( farPlaneSide.x, -farPlaneSide.y, farPlaneSide.z );
	localFrustumVertices[2] = Vector( -farPlaneSide.x, -farPlaneSide.y, farPlaneSide.z );
	localFrustumVertices[3] = Vector( -farPlaneSide.x, farPlaneSide.y, farPlaneSide.z );
}

void Viewport::_updateFrustum()
{
	worldFrustumVertices[0] = getWorldPosition( localFrustumVertices[0] );
	worldFrustumVertices[1] = getWorldPosition( localFrustumVertices[1] );
	worldFrustumVertices[2] = getWorldPosition( localFrustumVertices[2] );
	worldFrustumVertices[3] = getWorldPosition( localFrustumVertices[3] );
    
    Vector worldPosition = getWorldPosition();

	worldFrustumPlanes[0].setup( worldFrustumVertices[1], worldFrustumVertices[0], worldPosition );
	worldFrustumPlanes[1].setup( worldFrustumVertices[2], worldFrustumVertices[1], worldPosition );
	worldFrustumPlanes[2].setup( worldFrustumVertices[3], worldFrustumVertices[2], worldPosition );
	worldFrustumPlanes[3].setup( worldFrustumVertices[0], worldFrustumVertices[3], worldPosition ); 
	//far plane
	worldFrustumPlanes[0].setup( worldFrustumVertices[1], worldFrustumVertices[2], worldFrustumVertices[0] );
}

void Viewport::_updateTransforms()
{
    //compute view, that is the inverse of our world matrix    
    mViewTransform = Matrix(1);
    
    Vector worldPos = getWorldPosition();
    
    glm::vec3 t( -worldPos.x, -worldPos.y, -worldPos.z );
    
    mViewTransform = glm::scale( mViewTransform, Vector( 1.f/scale.x, 1.f/scale.y, 1.f/scale.z ) );
    mViewTransform *= glm::mat4_cast( glm::inverse( rotation ) );
    mViewTransform = glm::translate( mViewTransform, t );
    
    //compute ortho projection
    mOrthoTransform = glm::ortho(-getHalfSize().x, 
                              getHalfSize().x,
                              -getHalfSize().y,
                              getHalfSize().y,
                                 zNear,
                                 zFar ) * mViewTransform;
       
    //compute frustum projection
}

bool Viewport::isContainedInFrustum( Renderable* r )
{
	DEBUG_ASSERT( r );

	//for each plane, check where the AABB is placed
	for( uint i = 0; i < 5; ++i )
	{
		Vector bounds = r->getWorldMax() - r->getWorldMin();

		if( worldFrustumPlanes[i].getSide( r->getWorldPosition(), bounds ) == -1 )
			return false;
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

