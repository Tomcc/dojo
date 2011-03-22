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
	Vector offset;
	offset.z = zFar;
	offset.y = zFar * tan( Math::toRadian( VFOV*0.5 ) );
	offset.x = ((float)targetSize.x/(float)targetSize.y) * offset.y;

	localFrustumVertices[0] = Vector( offset.x, offset.y, offset.z );
	localFrustumVertices[1] = Vector( offset.x, -offset.y, offset.z );
	localFrustumVertices[2] = Vector( -offset.x, -offset.y, offset.z );
	localFrustumVertices[3] = Vector( -offset.x, offset.y, offset.z );
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
	Vector localPos = getLocalPosition( pos );

	return Vector(
		(localPos.x - localFrustumVertices[2].x)/(localFrustumVertices[0].x-localFrustumVertices[2].x)*targetSize.x,
		(localPos.y - localFrustumVertices[2].y)/(localFrustumVertices[0].y-localFrustumVertices[2].y)*targetSize.y,
		0
		);
}

void Viewport::makeScreenSize( Vector& dest, int w, int h )
{	
	dest.x = (float)w/targetSize.x * size.x * nativeToScreenRatio;
	dest.y = (float)h/targetSize.y * size.y * nativeToScreenRatio;
}

