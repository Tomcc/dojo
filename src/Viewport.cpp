#include "stdafx.h"

#include "Viewport.h"

#include "Render.h"

using namespace Dojo;

void Viewport::Plane::setup( const Vector& center, const Vector& A, const Vector& B )
{
	n = (A-center) ^ (B-center);
	n.normalize();

	d = -(center * n);
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
	farPlaneSide.y = zFar * tan( Math::toRadian( VFOV*0.5 ) );
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

	worldFrustumPlanes[0].setup( worldFrustumVertices[1], worldFrustumVertices[0], worldPosition );
	worldFrustumPlanes[1].setup( worldFrustumVertices[2], worldFrustumVertices[1], worldPosition );
	worldFrustumPlanes[2].setup( worldFrustumVertices[3], worldFrustumVertices[2], worldPosition );
	worldFrustumPlanes[3].setup( worldFrustumVertices[0], worldFrustumVertices[3], worldPosition ); 
	//far plane
	worldFrustumPlanes[0].setup( worldFrustumVertices[1], worldFrustumVertices[2], worldFrustumVertices[0] );
}

bool Viewport::isContainedInFrustum( Renderable* r )
{
	DEBUG_ASSERT( r );

	//for each plane, check where the AABB is placed
	for( uint i = 0; i < 5; ++i )
	{
		Vector bounds = r->getWorldMax() - r->getWorldMin();

		if( worldFrustumPlanes[i].getSide( r->getWorldPosition(), bounds * 0.5f ) == -1 )
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
	local.x = (local.x / farPlaneSide.x) * halfSize.x;
	local.y = (local.y / farPlaneSide.y) * halfSize.y;

	return local;
}

void Viewport::makeScreenSize( Vector& dest, int w, int h )
{	
	dest.x = (float)w/targetSize.x * size.x * nativeToScreenRatio;
	dest.y = (float)h/targetSize.y * size.y * nativeToScreenRatio;
}

