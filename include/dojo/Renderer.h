//
//  Render.h
//  NinjaTraining
//
//  Created by Tommaso Checchi on 4/23/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//
#pragma once

#include "dojo_common_header.h"

#include "Array.h"
#include "Color.h"
#include "Vector.h"
#include "RenderLayer.h"

namespace Dojo {
	
	class RenderState;
	class Renderable;
	class Texture;
	class Viewport;
	class Mesh;
	class Game;
	
	class Renderer 
	{	
	public:		
		///a struct that exposes current rendering parameters such as transforms
		struct CurrentState
		{
			Matrix view, world, projection, worldView, worldViewProjection;
			Vector viewDirection, targetDimension;

		} currentState;
						
		typedef std::vector< RenderLayer > LayerList;
		typedef std::vector< Viewport* > ViewportList;
		
		Renderer( int width, int height, Orientation renderOrientation );		
		
		~Renderer();		
						
		void addRenderable( Renderable& s, RenderLayer::ID layerID );
				
		void removeRenderable( Renderable& s );
		
		void removeAllRenderables();

		void removeViewport( const Viewport& v );

		void removeAllViewports();
		
		///completely removes all layers!
		void clearLayers();
		
		void addViewport( Viewport& v );
						
		void setInterfaceOrientation( Orientation o );

		void setDefaultAmbient( const Color& a );
		
		Orientation getInterfaceOrientation()
		{
			return renderOrientation;
		}
		
		RenderLayer& getLayer( RenderLayer::ID layerID );
		
		bool hasLayer( RenderLayer::ID layerID );

		int getLayerNumber()
		{
			return positiveLayers.size() + negativeLayers.size();
		}
		
		RenderLayer::ID getBottomLayerID() const
		{
			return -(RenderLayer::ID)negativeLayers.size();	
		}
		
		RenderLayer::ID getFrontLayerID() const
		{
			return positiveLayers.size();
		}

		int getLastFrameVertexCount()		{	return frameVertexCount;	}
		int getLastFrameTriCount()			{	return frameTriCount;		}
		int getLastFrameBatchCount()		{	return frameBatchCount;		}

		const Color& getDefaultAmbient()			{	return defaultAmbient;		}
		
		bool isValid()						{	return valid;		}
		
		///renders a single element using the given viewport
		void renderElement( Viewport& viewport, Renderable& elem );
		
		///renders a whole layer on the given viewport
		void renderLayer( Viewport& viewport, const RenderLayer& layer );

		///renders a viewport and all its visible layers
		void renderViewport( Viewport& viewport );
		
		//renders all the layers and their contained Renderables in the given order
		void render();
				
	protected:
		
		bool valid;
						
		// The pixel dimensions of the target system view
		int width, height;
		
		float renderRotation;
		Orientation renderOrientation, deviceOrientation;

		ViewportList viewportList;
		
		const RenderLayer* currentLayer;

		int frameVertexCount, frameTriCount, frameBatchCount;
				
		bool frameStarted;
		
		LayerList negativeLayers, positiveLayers;
	
		Color defaultAmbient;
		
		Matrix mRenderRotation;
	};		
}

