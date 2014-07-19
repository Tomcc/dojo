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
			bool visible,
				depthCheck,
				lightingOn,
				projectionOff,
				depthClear,
				wireframe;

			Layer() :
			visible( true ),
			depthCheck( false ),
			lightingOn( false ),
			projectionOff( true ),
			depthClear( true ),
			wireframe( false )
			{

			}
		};

		///a struct that exposes current rendering parameters such as transforms
		struct CurrentState
		{
			Matrix view, world, projection, worldView, worldViewProjection;
			Vector viewDirection, targetDimension;

		} currentState;
						
		typedef Array< Layer* > LayerList;
		typedef Array< Light* > LightList;
		typedef Array< Viewport* > ViewportList;
		
		Render( int width, int height, Orientation renderOrientation );		
		
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

		void removeViewport( Viewport* v )
		{
			mViewportList.remove( v );
		}

		void removeAllViewports()
		{
			mViewportList.clear();
		}
		
		void addLight( Light* l )
		{
			DEBUG_ASSERT( l, "addLight: null light passed" );
			DEBUG_ASSERT( lights.size() < RENDER_MAX_LIGHTS, "addLight: Cannot add this light as it is past the supported light number (RENDER_MAX_LIGHTS)" );
			
			lights.add( l );
		}
		
		void removeLight( Light* l )
		{
			DEBUG_ASSERT( l, "removeLight: null light passed" );
			
			lights.remove( l );	
			
			//remove removes always the last element in the list - just disable the last index now
			glDisable( GL_LIGHT0 + lights.size() );
		}
		
		///completely removes all layers!
		void clearLayers()
		{
			for( int i = 0; i < negativeLayers.size(); ++i )
				SAFE_DELETE( negativeLayers[i] );
			
			negativeLayers.clear();
			
			for( int i = 0; i < positiveLayers.size(); ++i )
				SAFE_DELETE( positiveLayers[i] );
			
			positiveLayers.clear();
		}
		
		void addViewport( Viewport* v );
						
		void setInterfaceOrientation( Orientation o );

		void setDefaultAmbient( const Color& a )
		{
			defaultAmbient = a;
			defaultAmbient.a = 1;
		}
		
		Orientation getInterfaceOrientation()
		{
			return renderOrientation;
		}
		
		Layer* getLayer( int layerID );
		
		bool hasLayer( int layerID );

		int getLayerNumber()
		{
			return positiveLayers.size() + negativeLayers.size();
		}
		
		int getFirstLayerID()
		{
			return -negativeLayers.size();	
		}
		
		int getLastLayerID()
		{
			return positiveLayers.size();
		}

		int getLastFrameVertexCount()		{	return frameVertexCount;	}
		int getLastFrameTriCount()			{	return frameTriCount;		}
		int getLastFrameBatchCount()		{	return frameBatchCount;		}

		const Color& getDefaultAmbient()			{	return defaultAmbient;		}
		
		bool isValid()						{	return valid;		}
		
		///renders a single element using the given viewport
		void renderElement( Viewport* v, Renderable* r );
		
		///renders a whole layer on the given viewport
		void renderLayer( Viewport* v, Layer* list );

		///renders a viewport and all its visible layers
		void renderViewport( Viewport* v );
		
		//renders all the layers and their contained Renderables in the given order
		void render();
				
	protected:	

		Platform* platform;
		
		bool valid;
						
		// The pixel dimensions of the target system view
		int width, height;
		
		float renderRotation;
		Orientation renderOrientation, deviceOrientation;

		ViewportList mViewportList;
		
		RenderState* currentRenderState, *firstRenderState;
		Layer* currentLayer;

		int frameVertexCount, frameTriCount, frameBatchCount;
				
		bool frameStarted;
		
		LayerList negativeLayers, positiveLayers;
		Layer* backLayer;
		
		LightList lights;
		Color defaultAmbient;
		
		Matrix mRenderRotation;
	};		
}

#endif