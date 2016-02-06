/*
 *  Renderable.h
 *  Dojo Training
 *
 *  Created by Tommaso Checchi on 7/14/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "RenderState.h"
#include "Color.h"
#include "AABB.h"
#include "RenderLayer.h"
#include "Component.h"

namespace Dojo {
	class Object;
	class Renderer;
	class GameState;

	class Renderable :
		public Component,
		public RenderState {
	public:
		static const int ID = 0;

		Vector uvOffset;
		Vector scale = Vector::One;

		Renderable(Object& parent, RenderLayer::ID layer);

		Renderable(Object& parent, RenderLayer::ID layer, Mesh& m, Shader& shader);

		Renderable(Object& parent, RenderLayer::ID layer, const utf::string& meshName, const utf::string& shaderName);

		virtual ~Renderable();

		GameState& getGameState() const;

		void setVisible(bool v) {
			visible = v;
		}

		///starts a linear fade on the color of this Renderable, from start to end and "duration" seconds long
		void startFade(const Color& start, const Color& end, float duration);

		///starts a linear fade on the alpha of this Renderable, from start to end and "duration" seconds long
		void startFade(float startAlpha, float endAlpha, float duration);

		///stops a current fade leaving the Renderable in the current state
		void stopFade();

		///returns the ID of the Render::Layer this object is assigned to
		RenderLayer::ID getLayer() const {
			return layer;
		}

		const AABB& getGraphicsAABB() const {
			return mWorldBB;
		}

		///true if this object has been assigned to a Render::Layer
		bool hasLayer() {
			return layer != RenderLayer::InvalidID;
		}

		///tells if the object is either visible or has a mesh
		bool isVisible() const {
			return visible;
		}

		bool canBeRendered() const;

		bool isFading() const {
			return fading;
		}

		void advanceFade(float dt);

		virtual void update(float dt);

		virtual void onAttach() override;
		virtual void onDetach() override;
	protected:

		bool visible = true;

		RenderLayer::ID layer;

		bool fading = false;
		float currentFadeTime = 0;
		float fadeEndTime;
		Color fadeStartColor;
		Color fadeEndColor;

		AABB mWorldBB;
	};
}
