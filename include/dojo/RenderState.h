/*
 *  RenderState.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/12/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "Color.h"
#include "Vector.h"
#include "BlendingMode.h"
#include "Radians.h"

namespace Dojo {
	class Texture;
	class Mesh;
	class Shader;
	class GlobalUniformData;

	///A render state is responsibile of managing the state of the underlying OGL render minimising the changes to be done when it is activated
	class RenderState {
	public:

		enum class CullMode {
			Front,
			Back,
			None
		};

		Color color;

		GLenum srcBlend, destBlend, blendFunction;

		CullMode cullMode;

		RenderState();

		virtual ~RenderState();

		void setMesh(Mesh& m) {
			mesh = m;
		}

		///Sets a texture in the required slot.
		/**
		It can be nullptr, which means that the slot is disabled.
		*/
		void setTexture(optional_ref<Texture> tex, int ID = 0);

		///enables or disables blending of this RS
		void setBlendingEnabled(bool enabled) {
			blendingEnabled = enabled;
		}

		///sets the blending mode with a single call
		void setBlending(GLenum src, GLenum dest) {
			srcBlend = src;
			destBlend = dest;
		}

		///sets an abstract photoshop-like blending mode
		void setBlending(BlendingMode mode);

		///sets the Shader material to be used for this RenderState
		void setShader(Shader& shader);

		optional_ref<Texture> getTexture(int ID = 0) const;

		///returns the Mesh currently used by this state
		optional_ref<Mesh> getMesh() const {
			return mesh;
		}

		///returns the Shader currently bound to this state
		optional_ref<Shader> getShader() const {
			return mShader;
		}

		const Matrix& getTransform() const {
			return mTransform;
		}

		///returns the "weight" of the changes needed to pass from "this" to "s"
		int getDistance(RenderState* s);

		void apply(const GlobalUniformData& currentState) const;

		void applyStateDiff(const GlobalUniformData& currentState, optional_ref<const RenderState> lastState) const;

	protected:

		bool blendingEnabled;

		optional_ref<Mesh> mesh;
		optional_ref<Shader> mShader;
		optional_ref<Texture> textures[DOJO_MAX_TEXTURES];
		int maxTextureSlot = 0;

		Matrix mTransform;

		void _bindTextureSlot(int i);
	};
}
