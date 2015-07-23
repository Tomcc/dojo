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

	///A render state is responsibile of managing the state of the underlying OGL render minimising the changes to be done when it is activated
	class RenderState {
	public:
		class TextureUnit {
		public:

			Texture* texture;

			TextureUnit();
			explicit TextureUnit(Texture* t);

			~TextureUnit();

			void applyTransform();

			void setOffset(const Vector& v) {
				offset = v;
				hasTextureTransform = true;
			}

			void setScale(const Vector& v) {
				scale = v;
				hasTextureTransform = true;
			}

			void setRotation(const Radians r) {
				rotation = r;
				hasTextureTransform = true;
			}

			const Vector& getOffset() {
				return offset;
			}

			const Vector& getScale() {
				return scale;
			}

			Radians getRotation() const {
				return rotation;
			}

			Matrix getTransform() const;

			bool isTransformRequired() const {
				return hasTextureTransform;
			}

		protected:

			Vector offset, scale;
			Radians rotation;
			bool hasTextureTransform = false;
		};

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
			mesh = &m;
		}

		///Sets a texture in the required slot.
		/**
		It can be NULL, which means that the slot is disabled.
		*/
		void setTexture(Texture* tex, int ID = 0);

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
		/**
		\remark the shader may be null to remove shader use
		*/
		void setShader(Shader* shader);

		Texture* getTexture(int ID = 0) const;

		const TextureUnit& getTextureUnit(int ID) const;

		///returns the Mesh currently used by this state
		Mesh* getMesh() const {
			return mesh;
		}

		///returns the Shader currently bound to this state
		Shader* getShader() {
			return pShader;
		}

		///returns the "weight" of the changes needed to pass from "this" to "s"
		int getDistance(RenderState* s);

		void applyState();

		void commitChanges();

	protected:

		bool blendingEnabled;

		TextureUnit textures[ DOJO_MAX_TEXTURES ];

		Mesh* mesh = nullptr;

		Shader* pShader;

		void _bindTextureSlot(int i);
	};
}
