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

#include "Object.h"
#include "Plane.h"
#include "Vector.h"
#include "Color.h"
#include "Renderable.h"

namespace Dojo 
{	
	class Render;
	class GameState;
	class AnimatedQuad;
	class Model;
	class Texture;
		
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
			int fadeObjectLayer = 8 );
		
		virtual ~Viewport()
		{
			
		}	


		///enable this viewport for frustum culling, setting the frustum values
		void enableFrustum( float VFOV, float zNear, float zFar );
		
		void setBackgroundSprite( const String& name, float frameTime = 0 );
				
		inline void setClearColor( const Color& color)	{	clearColor = color;	}	

		inline void setTargetSize( const Vector& size )		{ targetSize = size; }

		inline void setCullingEnabled( bool state )		{	cullingEnabled = state;	}
		
		inline const Color& getClearColor()				{	return clearColor;	}
		inline AnimatedQuad* getBackgroundSprite()		{	return background;	}
		inline Model* getFader()						{	return fadeObject;	}
		inline float getVFOV()							{	return VFOV;		}
		inline float getZFar()							{	return zFar;		}
		inline float getZNear()							{	return zNear;		}
		inline const Vector* getWorldFrustumVertices()	{	return worldFrustumVertices;	}
		inline const Vector* getLocalFrustumVertices()	{	return localFrustumVertices;	}
		inline const Vector& getTargetSize()			{   return targetSize;  }

		Vector getScreenPosition( const Vector& pos );
		
		///given a [0,1] normalized SS pos, returns the direction of the world space ray it originates
		Vector getRayDirection( const Vector& screenSpacePos );
        
        inline const Matrix& getViewProjOrtho()
        {
            return mOrthoTransform;
        }
        inline const Matrix& getViewProjFrustum()
        {
            return mFrustumTransform;
        }

		bool isContainedInFrustum( Renderable* r );

		inline bool isSeeing( Renderable* s )
		{
			DEBUG_ASSERT( s );

			return cullingEnabled && s->isVisible() && touches( s );
		}

		inline bool touches( Renderable* r )
		{
			DEBUG_ASSERT( r );

			return Math::AABBsCollide( r->getWorldMax(), r->getWorldMin(), getWorldMax(), getWorldMin() );
		}
		
		inline Vector makeWorldCoordinates( const Vector& screenPoint )
		{
			return makeWorldCoordinates( (int)screenPoint.x, (int)screenPoint.y );
		}
		
		inline Vector makeWorldCoordinates( int x, int y )
		{
			return Vector(
						  getWorldMin().x + ((float)x / targetSize.x) * size.x,
						  getWorldMax().y - ((float)y / targetSize.y) * size.y );
		}
			
		inline void makeScreenSize( Vector& dest, int w, int h )
		{	
			dest.x = ((float)w/targetSize.x) * size.x;// * nativeToScreenRatio;
			dest.y = ((float)h/targetSize.y) * size.y;// * nativeToScreenRatio;
		}
				
		void makeScreenSize( Vector& dest, Texture* tex );
		
		inline float getPixelSide()
		{
			return size.x / targetSize.x;
		}

		virtual void onAction( float dt )
		{
			Object::onAction(dt);
            
            _updateTransforms();

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
        
        Matrix mViewTransform, mOrthoTransform, mFrustumTransform;

		float nativeToScreenRatio;

		//frustum data
		bool frustumCullingEnabled;

		Vector localFrustumVertices[4];
		Vector worldFrustumVertices[4];

		Plane worldFrustumPlanes[5];

		float VFOV, zNear, zFar;
		Vector farPlaneSide;

		void _updateFrustum();
        void _updateTransforms();
	};
}

#endif

