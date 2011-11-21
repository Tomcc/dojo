//
//  Render.h
//  NinjaTraining
//
//  Created by Tommaso Checchi on 4/23/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//
#ifndef Render_h__
#define Render_h__

#include "dojo_common_header.h"

#include "Array.h"
#include "Color.h"
#include "Vector.h"

#define RENDER_MAX_LIGHTS 8

namespace Dojo {
	
	class RenderState;
	class Renderable;
	class Texture;
	class Viewport;
	class Mesh;
	class Platform;
	class Game;
	class Light;
	
	class Render 
	{	
	public:		

		class Layer : public Dojo::Array< Renderable* >
		{
		public:
			bool depthCheck;
			bool lightingOn;
			bool projectionOff;
			bool depthClear;

			Layer() :
			depthCheck( false ),
			lightingOn( false ),
			projectionOff( true ),
			depthClear( true )
			{

			}
		};
		
		enum RenderOrientation
		{
			RO_PORTRAIT,
			RO_PORTRAIT_REVERSE,
			RO_LANDSCAPE_LEFT,
			RO_LANDSCAPE_RIGHT
		};
						
		typedef Array< Layer* > LayerList;
		typedef Array< Light* > LightList;
		
		Render( uint width, uint height, uint devicePixelScale, RenderOrientation);		
		
		~Render();		
						
		void addRenderable( Renderable* s, int layer );
				
		void removeRenderable( Renderable* s );
		
		void removeAllRenderables()
		{
			for( int i = 0; i < negativeLayers.size(); ++i )
				negativeLayers.at(i)->clear();
			
			for( int i = 0; i < positiveLayers.size(); ++i )
				positiveLayers.at(i)->clear();
		}
		
		inline void addLight( Light* l )
		{
			DEBUG_ASSERT( l );
			DEBUG_ASSERT( lights.size() < RENDER_MAX_LIGHTS );
			
			lights.add( l );
		}
		
		inline void removeLight( Light* l )
		{
			DEBUG_ASSERT( l );
			
			lights.remove( l );	
			
			//remove removes always the last element in the list - just disable the last index now
			glDisable( GL_LIGHT0 + lights.size() );
		}
		
		///completely removes all layers!
		inline void clearLayers()
		{
			for( int i = 0; i < negativeLayers.size(); ++i )
				SAFE_DELETE( negativeLayers[i] );
			
			negativeLayers.clear();
			
			for( int i = 0; i < positiveLayers.size(); ++i )
				SAFE_DELETE( positiveLayers[i] );
			
			positiveLayers.clear();
		}
		
		void setViewport( Viewport* v );
						
		void setInterfaceOrientation( RenderOrientation o );

		void setWireframe( bool wireframe );
		
		inline void setDefaultAmbient( const Color& a )
		{
			defaultAmbient = a;
			defaultAmbient.a = 1;
		}
		
		inline RenderOrientation getInterfaceOrientation()
		{
			return renderOrientation;
		}
		
		Layer* getLayer( int layerID );
		
		bool hasLayer( int layerID );

		uint getLayerNumber()
		{
			return positiveLayers.size() + negativeLayers.size();
		}
		
		inline int getFirstLayerID()
		{
			return -negativeLayers.size();	
		}
		
		inline int getLastLayerID()
		{
			return positiveLayers.size();
		}
		
		inline int getScreenWidth()						{	return width;		}
		inline int getScreenHeight()					{	return height;		}
		
		inline int getWidth()
		{
			return viewportWidth;
		}
		
		inline int getHeight()
		{
			return viewportHeight;
		}
		
		inline float getContentScale()				{	return devicePixelScale;	}
		inline float getNativeToScreenRatio()		{	return nativeToScreenRatio;	}
		inline Viewport* getViewport()				{	return viewport;	}

		inline uint getLastFrameVertexCount()		{	return frameVertexCount;	}
		inline uint getLastFrameTriCount()			{	return frameTriCount;		}
		inline uint getLastFrameBatchCount()		{	return frameBatchCount;		}
		
		inline bool isValid()						{	return valid;		}
						
		void startFrame();
		
		void renderElement( Renderable* r );
		
		void renderLayer( Layer* list );
		
		void endFrame();
		
		//renders all the layers and their contained Renderables in the given order
		void render()
		{
			startFrame();

			//first render from the most negative to -1
			if( negativeLayers.size() > 0 )
			{				
				for( int i = negativeLayers.size()-1; i >= 0; --i )
					renderLayer( negativeLayers.at(i) );
			}

			//then from 0 to the most positive
			for( uint i = 0; i < positiveLayers.size(); ++i )
				renderLayer( positiveLayers.at(i) );

			endFrame();
		}
				
	protected:	

		Platform* platform;
		
		bool valid;
						
		// The pixel dimensions of the CAEAGLLayer
		int width, height, viewportWidth, viewportHeight;
		float devicePixelScale;
		
		float renderRotation;
		RenderOrientation renderOrientation, deviceOrientation;
		
		float nativeToScreenRatio;
						
		Viewport* viewport;	
		Vector viewportPixelRatio, textureScreenPixelRatio, spriteScreenPixelSize;
		
		RenderState* currentRenderState, *firstRenderState;
		Layer* currentLayer;

		int frameVertexCount, frameTriCount, frameBatchCount;
				
		bool frameStarted;
		
		LayerList negativeLayers, positiveLayers;
		Layer* backLayer;
		
		LightList lights;
		Color defaultAmbient;
		
		//precomputed matrices
		float orthoProj[16], orthoView[16], frustumProj[16], frustumView[16];

		void _setupOrthoProjection();
		void _setupFrustumProjection();
		
		void _gluPerspectiveClone( float fovy, float aspect, float zNear, float zFar);
		void _gluLookAtClone(float eyex, float eyey, float eyez,							 
							 float centerx, float centery, float centerz,							 
							 float upx, float upy, float upz);
	};		
}

#endif