/*
 *  Viewport.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/25/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "Object.h"
#include "Plane.h"
#include "Vector.h"
#include "Color.h"
#include "Renderable.h"
#include "Platform.h"

namespace Dojo 
{	
	class Renderer;
	class GameState;
	class AnimatedQuad;
	class Renderable;
	class RenderLayer;
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

		typedef std::vector< int > LayerList;
					
		Viewport( 
			Object* level, 
			const Vector& pos,
			const Vector& size, 
			const Color& clear, 
			float _VFOV = 0, 
			float _zNear = 0,
			float _zFar = 100 );
		
		virtual ~Viewport();

		///enable this viewport for frustum culling, setting the frustum values
		void enableFrustum( float VFOV, float zNear, float zFar );

		///adds a Fader object (fullscreen colored quad) at the given level to the Viewport
		void addFader( int layer );
		
		///orients the camera to look at a given 3D point
		void lookAt( const Vector& worldPos );
				
		void setClearColor( const Color& color)	{	clearColor = color;	}

		///sets the texture to be used as rendering target, null means "render to screen"
		void setRenderTarget( Texture* target );

		///sets which subset of Render Layers this Viewport is able to "see"
		void setVisibleLayers(const LayerList& layers);

		///sets which subset of Render Layers this Viewport is able to "see"
		void setVisibleLayers(int min, int max);

		///returns the subset of visible layers that has been set by setVisibleLayers
		/**
		by default, the set is empty, which means "all layers"
		*/
		const LayerList& getVisibleLayers() const
		{
			return mLayerList;
		}

		///set the pixel size of the "virtual rendering area"
		/**
		all the rendering is then scaled to have the virtual rendering area fit inside the real rendering area.
		This is useful when fitting a fixed-scale pixel-perfect scene inside a resizable window.
		*/
		void setTargetSize( const Vector& size )		{ targetSize = size; }
		
		const Color& getClearColor()				{	return clearColor;	}
		Renderable* getFader()					{	return faderObject;	}
		float getVFOV()							{	return VFOV;		}
		float getZFar()							{	return zFar;		}
		float getZNear()							{	return zNear;		}
		const Vector* getWorldFrustumVertices()	{	return worldFrustumVertices;	}
		const Vector* getLocalFrustumVertices()	{	return localFrustumVertices;	}
		const Vector& getTargetSize()			{   return targetSize;  }
		const Matrix& getViewTransform()			{	return mViewTransform;	}

		void setClearEnabled(bool enabled) {
			enableClear = enabled;
		}

		bool getClearEnabled() const {
			return enableClear;
		}

		///returns the Texture this Viewport draws to
		Texture* getRenderTarget()
		{
			return mRT;
		}

		///returns the on-screen position of the given world-space vector
		Vector getScreenPosition( const Vector& pos );
		
		///given a [0,1] normalized SS pos, returns the direction of the world space ray it originates
		Vector getRayDirection( const Vector& screenSpacePos );
        
        const Matrix& getOrthoProjectionTransform()
        {
            return mOrthoTransform;
        }
        const Matrix& getPerspectiveProjectionTransform()
        {
            return mFrustumTransform;
        }

		bool isContainedInFrustum( const Renderable& r ) const;

		bool isVisible( Renderable& s );

		bool isInViewRect( const Renderable& r ) const;
		
		///returns the world position of the given screenPoint
		Vector makeWorldCoordinates( const Vector& screenPoint )
		{
			return makeWorldCoordinates( (int)screenPoint.x, (int)screenPoint.y );
		}

		///returns the world position of the given screenPoint
		Vector makeWorldCoordinates( int x, int y );
			
		///converts the w and h pixel sizes in a screen space size
		void makeScreenSize( Vector& dest, int w, int h )
		{	
			dest.x = ((float)w/targetSize.x) * size.x;// * nativeToScreenRatio;
			dest.y = ((float)h/targetSize.y) * size.y;// * nativeToScreenRatio;
		}
		
		///converts the texture pixel sizes in a screen space size
		void makeScreenSize( Vector& dest, Texture* tex );
		
		float getPixelSide() const
		{
			return size.x / targetSize.x;
		}
        
        void setEyeTransform( const Matrix& t )
        {
            mPerspectiveEyeTransform = t;
        }

		virtual void onAction( float dt );
				
	protected:
		
		Vector targetSize;

		bool enableClear = true, frustumDirty = true;

		Matrix lastWorldTransform;
		
		Renderable* faderObject = nullptr;
				
		Color clearColor;
        
        Matrix mViewTransform, mOrthoTransform, mFrustumTransform, mPerspectiveEyeTransform;
		
		Vector localFrustumVertices[4];
		Vector worldFrustumVertices[4];

		Plane worldFrustumPlanes[5];

		float VFOV, zNear, zFar;
		Vector farPlaneSide;

		LayerList mLayerList;
		Texture* mRT;

		void _updateFrustum();
        void _updateTransforms();
	};
}

