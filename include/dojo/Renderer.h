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

#define RENDER_MAX_LIGHTS 8

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

		class Layer
		{
		public:
			typedef int ID;

			bool visible = true,
				depthCheck = false,
				orthographic = true,
				depthClear = true,
				wireframe = false;

			std::vector<Renderable*> elements;

			void remove(const Renderable& s);
		};

		///a struct that exposes current rendering parameters such as transforms
		struct CurrentState
		{
			Matrix view, world, projection, worldView, worldViewProjection;
			Vector viewDirection, targetDimension;

		} currentState;
						
		typedef std::vector< Layer > LayerList;
		typedef std::vector< Viewport* > ViewportList;
		
		Renderer( int width, int height, Orientation renderOrientation );		
		
		~Renderer();		
						
		void addRenderable( Renderable& s, Layer::ID layerID );
				
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
		
		Layer& getLayer( Layer::ID layerID );
		
		bool hasLayer( Layer::ID layerID );

		int getLayerNumber()
		{
			return positiveLayers.size() + negativeLayers.size();
		}
		
		Layer::ID getBottomLayerID() const
		{
			return -(Layer::ID)negativeLayers.size();	
		}
		
		Layer::ID getFrontLayerID() const
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
		void renderLayer( Viewport& viewport, const Layer& layer );

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
		
		const Layer* currentLayer;

		int frameVertexCount, frameTriCount, frameBatchCount;
				
		bool frameStarted;
		
		LayerList negativeLayers, positiveLayers;
	
		Color defaultAmbient;
		
		Matrix mRenderRotation;
	};		
}

