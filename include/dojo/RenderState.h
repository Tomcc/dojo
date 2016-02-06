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
		struct GLBlend {
			int src, dest, func;
			GLBlend(int src, int dest, int func, bool autoEnable = false) :
				src(src), 
				dest(dest), 
				func(func),
				autoEnable(autoEnable) {

			}

			GLBlend();

			bool isAuto() const {
				return autoEnable;
			}

		protected:
			bool autoEnable = false;
		};

		enum class CullMode {
			Front,
			Back,
			None
		};

		Color color;
		CullMode cullMode;

		RenderState();
		RenderState(const RenderState&) = delete;
		RenderState& operator=(const RenderState&) = delete;

		virtual ~RenderState();

		void setMesh(Mesh& m);

		///Sets a texture in the required slot.
		/**
		It can be nullptr, which means that the slot is disabled.
		*/
		void setTexture(optional_ref<Texture> tex, int ID = 0);

		///sets an abstract photoshop-like blending mode
		void setBlending(BlendingMode mode);

		bool isBlendingEnabled() const;

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

		void apply(const GlobalUniformData& currentState, optional_ref<const RenderState> lastState) const;

	protected:
		GLBlend blending;

		optional_ref<Mesh> mesh;
		optional_ref<Shader> mShader;
		optional_ref<Texture> textures[DOJO_MAX_TEXTURES];
		bool mTransparency = false;
		int maxTextureSlot = 0;

		Matrix mTransform;

		void _bindTextureSlot(int i);

		void _updateTransparency();
	};
}
