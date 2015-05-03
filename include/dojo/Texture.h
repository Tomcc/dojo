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

namespace Dojo {
	class Mesh;
	class FrameSet;

	///A Texture is the image container in Dojo; all the images to be displayed need to be loaded in GPU memory using one
	class Texture : public Resource {
	public:
		///Create a empty new texture
		Texture(ResourceGroup* creator = NULL);

		///Create a empty new texture that will be loaded from path
		Texture(ResourceGroup* creator, const String& path);

		virtual ~Texture();

		///loads an empty texture with the given properties
		bool loadEmpty(int width, int height, PixelFormat destFormat);

		///loads the texture from a memory area with RGBA8 format
		bool loadFromMemory(const byte* imageData, int width, int height, PixelFormat sourceFormat, PixelFormat destFormat);

		///loads the texture from the image pointed by the filename
		bool loadFromFile(const String& path);

		///loads the texture from the given area in a Texture Atlas, without duplicating data
		/** 
		a texture of this kind is loaded via an .atlasinfo and doesn't use VRAM in itself */
		bool loadFromAtlas(Texture* tex, int x, int y, int sx, int sy);

		///loads the texture with the given parameters
		virtual bool onLoad();

		virtual void onUnload(bool soft = false);

		bool isLoaded() {
			return loaded;
		}

		///internal - binds this texture as the current GL active one
		virtual void bind(GLuint index);

		///internal - binds this texture as the current Render Target
		void bindAsRenderTarget(bool useDepthBuffer);

		void enableBilinearFiltering();
		void disableBilinearFiltering();

		void enableMipmaps();
		void disableMipmaps();

		void enableAnisotropicFiltering(float level);
		void disableAnisotropicFiltering();

		///A tiled texture repeats when UV > 1 or < 0, while a clamped texture does not
		void enableTiling();
		///A tiled texture repeats when UV > 1 or < 0, while a clamped texture does not
		void disableTiling();

		int getWidth() {
			return width;
		}

		int getHeight() {
			return height;
		}

		///returns the texture size in the UV space of the parent atlas/padded image
		const Vector& getUVSize() {
			return UVSize;
		}

		///returns the texture offset in the UV space of the parent atlas/padded image
		const Vector& getUVOffset() {
			return UVOffset;
		}

		int getInternalWidth() {
			return internalWidth;
		}

		int getInternalHeight() {
			return internalHeight;
		}

		///Returns a parent atlas Texture if this texture is a "fake" tile atlas
		Texture* getParentAtlas() {
			return parentAtlas;
		}

		///returns the FrameSet that will load and delete this Texture
		FrameSet* getOwnerFrameSet() {
			return ownerFrameSet;
		}

		///returns the size this texture would have on-screen if drawn in a pixel-perfect way
		const Vector& getScreenSize() {
			return screenSize;
		}

		///obtain the optimal billboard to use this texture as a sprite, when the device does not support Power of 2 Textures
		Mesh& getOptimalBillboard() {
			if (!OBB)
				_rebuildOptimalBillboard();

			return *OBB;
		}

		bool isNonPowerOfTwo() {
			return npot;
		}

		///true if it belongs to an atlas
		bool isAtlasTile() {
			return parentAtlas != NULL;
		}

		void _notifyScreenSize(const Vector& ss) {
			screenSize.x = ss.x;
			screenSize.y = ss.y;
		}

		void _notifyOwnerFrameSet(FrameSet* s) {
			DEBUG_ASSERT( ownerFrameSet == NULL, "Tried to set an owner on an already owned Texture" );
			DEBUG_ASSERT( s, "Tried to set a NULL owner on a Texture");

			ownerFrameSet = s;
		}

	protected:

		bool npot, mMipmapsEnabled;
		int width, height, internalWidth, internalHeight;
		GLenum internalFormat;
		Vector UVSize, UVOffset;

		Texture* parentAtlas;
		FrameSet* ownerFrameSet;
		int mAtlasOriginX, mAtlasOriginY;

		Unique<Mesh> OBB;

		GLuint glhandle, mDepthBuffer;

		Vector screenSize;

		GLuint mFBO;

		///builds the optimal billboard for this texture, used in AnimatedQuads
		void _rebuildOptimalBillboard();

		bool _setupAtlas();
	};
}
