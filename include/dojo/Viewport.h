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
	class Renderable;
	class Texture;
		
	///A Viewport is a View in a Dojo GameState, working both in 2D and 3D
	/**
	when rendering a 2D layer, the Viewport uses an orthogonal transform and manages
	the pixel-perfect rendering with its targetSize property;
	when rendering a 3D layer, it culls the scene and renders the perspective using its Frustum
	*/
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
			float _zFar = 100 );
		
		virtual ~Viewport()
		{
			
		}	


		///enable this viewport for frustum culling, setting the frustum values
		void enableFrustum( float VFOV, float zNear, float zFar );

		///adds a Fader object (fullscreen colored quad) at the given level to the Viewport
		void addFader( int layer );
		
		void setBackgroundSprite( const String& name, float frameTime = 0 );
				
		inline void setClearColor( const Color& color)	{	clearColor = color;	}	

		///set the pixel size of the "virtual rendering area"
		/**
		all the rendering is then scaled to have the virtual rendering area fit inside the real rendering area.
		This is useful when fitting a fixed-scale pixel-perfect scene inside a resizable window.
		*/
		inline void setTargetSize( const Vector& size )		{ targetSize = size; }

		///enables or disables 3D culling of hidden objects
		inline void setCullingEnabled( bool state )		{	cullingEnabled = state;	}
		
		inline const Color& getClearColor()				{	return clearColor;	}
		inline AnimatedQuad* getBackgroundSprite()		{	return background;	}
		inline Renderable* getFader()					{	return fadeObject;	}
		inline float getVFOV()							{	return VFOV;		}
		inline float getZFar()							{	return zFar;		}
		inline float getZNear()							{	return zNear;		}
		inline const Vector* getWorldFrustumVertices()	{	return worldFrustumVertices;	}
		inline const Vector* getLocalFrustumVertices()	{	return localFrustumVertices;	}
		inline const Vector& getTargetSize()			{   return targetSize;  }

		///returns the on-screen position of the given world-space vector
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
		
		///returns the world position of the given screenPoint
		inline Vector makeWorldCoordinates( const Vector& screenPoint )
		{
			return makeWorldCoordinates( (int)screenPoint.x, (int)screenPoint.y );
		}

		///returns the world position of the given screenPoint
		inline Vector makeWorldCoordinates( int x, int y )
		{
			return Vector(
						  getWorldMin().x + ((float)x / targetSize.x) * size.x,
						  getWorldMax().y - ((float)y / targetSize.y) * size.y );
		}
			
		///converts the w and h pixel sizes in a screen space size
		inline void makeScreenSize( Vector& dest, int w, int h )
		{	
			dest.x = ((float)w/targetSize.x) * size.x;// * nativeToScreenRatio;
			dest.y = ((float)h/targetSize.y) * size.y;// * nativeToScreenRatio;
		}
		
		///converts the texture pixel sizes in a screen space size
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
		Renderable* fadeObject;
				
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

