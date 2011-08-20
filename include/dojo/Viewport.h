/*
 *  Viewport.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/25/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef Viewport_h__
#define Viewport_h__

#include "dojo_common_header.h"

#include "Vector.h"
#include "Color.h"
#include "Object.h"
#include "Model.h"
#include "Sprite.h"
#include "Texture.h"
#include "Platform.h"
#include "Renderable.h"
#include "Plane.h"

namespace Dojo 
{	
	class Render;
	
	class Viewport : public Object
	{
	public:
					
		Viewport( 
			GameState* level, 
			const Vector& pos,
			const Vector& size, 
			const Color& clear, 
			float _VFOV = 0, 
			float _zNear = 0,
			float _zFar = 100,
			int fadeObjectLayer = 8 ) :
		Object( level, pos, size ),
		clearColor( clear ),
		cullingEnabled( true ),
		background( NULL ),
		VFOV( 0 ),
		zNear( 0 ),
		zFar( 1000 ),
		frustumCullingEnabled( false )
		{
			Render* render = Platform::getSingleton()->getRender();

			nativeToScreenRatio = render->getNativeToScreenRatio();
			
			targetSize.x = (float)render->getWidth();
			targetSize.y = (float)render->getHeight();

			//create the fader object			
			fadeObject = new Model( level, Vector::ZERO, "texturedQuad", String::EMPTY );
			fadeObject->color = Color( 0, 0, 0, 0 );
			
			fadeObject->scale.x = size.x;
			fadeObject->scale.y = size.y;
			
			fadeObject->setVisible( false );
			fadeObject->inheritAngle = false;

			addChild( fadeObject, fadeObjectLayer, false );

			if( _VFOV > 0 )
				enableFrustum( _VFOV, _zNear, _zFar );
		}		
						
		virtual ~Viewport()
		{
			
		}	


		///enable this viewport for frustum culling, setting the frustum values
		void enableFrustum( float VFOV, float zNear, float zFar );
		
		inline void setBackgroundSprite( const String& name, float frameTime = 0 )
		{			
			DEBUG_ASSERT( name.size() );
			
			if( background )
			{
				removeChild( background );
				delete background;
			}
			
			background = new Sprite( gameState, Vector::ZERO, name, frameTime );
			background->setRequiresAlpha( false );
			background->setVisible( true );
			background->inheritAngle = false;
									
			//force the proportions to fill screen
			background->_updateScreenSize();
			
			//the background image must not be stretched on different aspect ratios
			//so we just pick the pixel size for the horizontal			
			Vector ss = background->getScreenSize();
			
			background->pixelScale.x = size.x / ss.x;
			background->pixelScale.y = background->pixelScale.x;	

			addChild( background );
		}
				
		inline void setClearColor( const Color& color)	{	clearColor = color;	}	

		inline void setCullingEnabled( bool state )		{	cullingEnabled = state;	}
		
		inline const Color& getClearColor()				{	return clearColor;	}
		inline AnimatedQuad* getBackgroundSprite()		{	return background;	}
		inline Model* getFader()						{	return fadeObject;	}
		inline float getVFOV()							{	return VFOV;		}
		inline float getZFar()							{	return zFar;		}
		inline float getZNear()							{	return zNear;		}
		inline const Vector* getWorldFrustumVertices()	{	return worldFrustumVertices;	}
		inline const Vector* getLocalFrustumVertices()	{	return localFrustumVertices;	}

		Vector getScreenPosition( const Vector& pos );
		
		///given a [0,1] normalized SS pos, returns the direction of the world space ray it originates
		Vector getRayDirecton( const Vector& screenSpacePos );

		bool isContainedInFrustum( Renderable* r );

		inline bool isSeeing( Renderable* s )
		{
			DEBUG_ASSERT( s );

			return cullingEnabled && s->isVisible() && touches( s );
		}

		inline bool touches( Renderable* r )
		{
			DEBUG_ASSERT( r );

			return Math::AABBsCollide( r->getAABBMax(), r->getAABBMin(), getWorldMax(), getWorldMin() );
		}
		
		inline Vector makeWorldCoordinates( const Vector& screenPoint )
		{
			return makeWorldCoordinates( (int)screenPoint.x, (int)screenPoint.y );
		}
		
		inline Vector makeWorldCoordinates( int x, int y )
		{
			return Vector(
						  getWorldMin().x + (x / targetSize.x) * size.x,
						  getWorldMax().y - (y / targetSize.y) * size.y );
		}
						
		void makeScreenSize( Vector& dest, int w, int h );
		
		inline void makeScreenSize( Vector& dest, Texture* tex )
		{
			makeScreenSize( dest, tex->getWidth(), tex->getHeight() );
		}

		virtual void onAction( float dt )
		{
			Object::onAction(dt);

			//do not call if not explicitly required
			if( frustumCullingEnabled )
				_updateFrustum();
		}
				
	protected:
		
		Vector targetSize;

		bool cullingEnabled;	
		
		AnimatedQuad* background;
		Model* fadeObject;
				
		Color clearColor;

		float nativeToScreenRatio;

		//frustum data
		bool frustumCullingEnabled;

		Vector localFrustumVertices[4];
		Vector worldFrustumVertices[4];

		Plane worldFrustumPlanes[5];

		float VFOV, zNear, zFar;
		Vector farPlaneSide;

		void _updateFrustum();
	};
}

#endif

