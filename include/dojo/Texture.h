/*
 *  Texture.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "Resource.h"
#include "Vector.h"
#include "PixelFormat.h"
#include "RenderSurface.h"

namespace Dojo {
	class Mesh;
	class FrameSet;

	///A Texture is the image container in Dojo; all the images to be displayed need to be loaded in GPU memory using one
	class Texture : 
		public RenderSurface,
		public Resource {
	public:
		///Create a empty new texture
		Texture(optional_ref<ResourceGroup> creator = {});

		///Create a empty new texture that will be loaded from path
		Texture(optional_ref<ResourceGroup> creator, const utf::string& path);

		virtual ~Texture();

		///loads an empty texture with the given properties
		bool loadEmpty(uint32_t width, uint32_t height, PixelFormat destFormat);

		///loads the texture from a memory area with RGBA8 format
		bool loadFromMemory(const uint8_t* imageData, uint32_t width, uint32_t height, PixelFormat sourceFormat);

		///loads the texture from the image pointed by the filename
		bool loadFromFile(const utf::string& path);

		///loads the texture from the given area in a Texture Atlas, without duplicating data
		/**
		a texture of this kind is loaded via an .atlasinfo and doesn't use VRAM in itself */
		bool loadFromAtlas(Texture& tex, int x, int y, int sx, int sy);

		///loads the texture with the given parameters
		virtual bool onLoad();

		virtual void onUnload(bool soft = false);

		bool isLoaded() {
			return loaded;
		}

		///internal - binds this texture as the current GL active one
		virtual void bind(uint32_t index);

		void enableBilinearFiltering();
		void disableBilinearFiltering();

		void enableAnisotropicFiltering(float level);
		void disableAnisotropicFiltering();

		///A tiled texture repeats when UV > 1 or < 0, while a clamped texture does not
		void enableTiling();
		///A tiled texture repeats when UV > 1 or < 0, while a clamped texture does not
		void disableTiling();

		///returns the texture size in the UV space of the parent atlas/padded image
		const Vector& getUVSize() const {
			return UVSize;
		}

		///returns the texture offset in the UV space of the parent atlas/padded image
		const Vector& getUVOffset()  const {
			return UVOffset;
		}

		uint32_t getInternalWidth() const {
			return internalWidth;
		}

		uint32_t getInternalHeight()  const {
			return internalHeight;
		}
		
		///Returns a parent atlas Texture if this texture is a "fake" tile atlas
		optional_ref<Texture> getParentAtlas() {
			return parentAtlas;
		}

		///returns the FrameSet that will load and delete this Texture
		optional_ref<FrameSet> getOwnerFrameSet() {
			return ownerFrameSet;
		}

		///returns the size this texture would have on-screen if drawn in a pixel-perfect way
		const Vector& getScreenSize() {
			return screenSize;
		}

		///obtain the optimal billboard to use this texture as a sprite, when the device does not support Power of 2 Textures
		Mesh& getOptimalBillboard();

		bool hasTransparency() const {
			return mTransparency;
		}

		///true if it belongs to an atlas
		bool isAtlasTile() {
			return parentAtlas.is_some();
		}

		void _notifyScreenSize(const Vector& ss);

		void _notifyOwnerFrameSet(FrameSet& s);

		void _addAsAttachment(uint32_t index, uint32_t width, uint32_t height, uint8_t miplevel);

	protected:

		bool mTransparency = false;
		uint32_t internalWidth, internalHeight;
		Vector UVSize, UVOffset;

		optional_ref<Texture> parentAtlas;
		optional_ref<FrameSet> ownerFrameSet;
		int mAtlasOriginX, mAtlasOriginY;

		Unique<Mesh> OBB;

		uint32_t glhandle;

		Vector screenSize;

		///builds the optimal billboard for this texture, used in AnimatedQuads
		void _rebuildOptimalBillboard();

		bool _setupAtlas();
		bool _createStorage(uint32_t w, uint32_t h, PixelFormat formatID);
	};
}
