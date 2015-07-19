//
//  Render.h
//  NinjaTraining
//
//  Created by Tommaso Checchi on 4/23/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//
#pragma once

#include "dojo_common_header.h"

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

	class Renderer {
	public:
		///a struct that exposes current rendering parameters such as transforms
		struct CurrentState {
			Matrix view, world, projection, worldView, worldViewProjection;
			Vector viewDirection, targetDimension;

		} currentState;

		typedef std::vector<RenderLayer> LayerList;
		typedef std::unordered_set<Viewport*> ViewportSet;

		Renderer(int width, int height, Orientation renderOrientation);

		~Renderer();

		void addRenderable(Renderable& s);

		void removeRenderable(Renderable& s);

		void removeAllRenderables();

		void removeViewport(const Viewport& v);

		void removeAllViewports();

		///completely removes all layers!
		void clearLayers();

		void addViewport(Viewport& v);

		void setInterfaceOrientation(Orientation o);

		void setDefaultAmbient(const Color& a);

		Orientation getInterfaceOrientation() {
			return renderOrientation;
		}

		RenderLayer& getLayer(RenderLayer::ID layerID);

		bool hasLayer(RenderLayer::ID layerID);

		int getLayerCount() const {
			return layers.size();
		}

		RenderLayer::ID getFrontLayerID() const {
			return (RenderLayer::ID)layers.size();
		}

		int getLastFrameVertexCount() {
			return frameVertexCount;
		}

		int getLastFrameTriCount() {
			return frameTriCount;
		}

		int getLastFrameBatchCount() {
			return frameBatchCount;
		}

		const Color& getDefaultAmbient() {
			return defaultAmbient;
		}

		bool isValid() {
			return valid;
		}

		//renders all the layers and their contained Renderables in the given order
		void renderFrame(float dt);

	protected:

		bool valid;

		// The pixel dimensions of the target system view
		int width, height;

		Radians renderRotation = 0.0_rad;
		Orientation renderOrientation, deviceOrientation;

		ViewportSet viewportList;

		int frameVertexCount, frameTriCount, frameBatchCount;

		bool frameStarted;

		LayerList layers;

		Color defaultAmbient;

		Matrix mRenderRotation;

		void _updateRenderables(const LayerList& layers, float dt);

		///renders a single element using the given viewport
		void _renderElement(Renderable& elem);
		void _renderLayer(Viewport& viewport, const RenderLayer& layer);
		void _renderViewport(Viewport& viewport);

	};
}
