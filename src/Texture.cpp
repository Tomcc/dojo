#include "Texture.h"
#include "dojomath.h"
#include "Platform.h"
#include "ResourceGroup.h"
#include "Mesh.h"
#include "TexFormatInfo.h"

#include "dojo_gl_header.h"

using namespace Dojo;

Texture::Texture(optional_ref<ResourceGroup> creator) :
	Resource(creator),
	internalWidth(0),
	internalHeight(0),
	glhandle(0),
	mFBO(GL_NONE) {
	mTexturePart = self; //HACK
}

Texture::Texture(optional_ref<ResourceGroup> creator, const utf::string& path) :
	Resource(creator, path),
	internalWidth(0),
	internalHeight(0),
	glhandle(0),
	mFBO(GL_NONE) {
	mTexturePart = self; //HACK
}

Texture::~Texture() {
	OBB.reset();

	if (loaded) {
		onUnload();
	}
}

void Texture::bind(uint32_t index) {
	//create the gl texture if still not created!
	if (!glhandle) {
		glGenTextures(1, &glhandle);

		CHECK_GL_ERROR;

		DEBUG_ASSERT( glhandle, "OpenGL Error, no texture handle was generated" );
	}

	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, glhandle);
}

void Texture::bindAsRenderTarget(bool depthBuffer) {
	if (!mFBO) { //create a new RT on the fly at the first request
		bind(0);

		glGenFramebuffers(1, &mFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

		CHECK_GL_ERROR;

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glhandle, 0);

		CHECK_GL_ERROR;

		//create the depth attachment if needed
		if (depthBuffer) {
			glGenRenderbuffersEXT(1, &mDepthBuffer);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mDepthBuffer);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT16, width, height);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, mDepthBuffer);
		}

		CHECK_GL_ERROR;

		uint32_t status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		DEBUG_ASSERT( status == GL_FRAMEBUFFER_COMPLETE, "The framebuffer is incomplete" );
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	}
}

void Texture::enableAnisotropicFiltering(float level) {
	bind(0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, level);
}

void Texture::disableAnisotropicFiltering() {
	bind(0);
	glTexParameterf(GL_TEXTURE_2D, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, 0);
}

void Texture::enableBilinearFiltering() {
	bind(0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

}

void Texture::disableBilinearFiltering() {
	bind(0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Texture::enableTiling() {
	bind(0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Texture::disableTiling() {
	bind(0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

bool Texture::loadEmpty(int w, int h, PixelFormat formatID) {
	width = w;
	height = h;

	DEBUG_ASSERT( width > 0, "Width must be more than 0" );
	DEBUG_ASSERT( height > 0, "Height must be more than 0" );

	auto& formatInfo = TexFormatInfo::getFor(formatID);
	auto& oldFormat = TexFormatInfo::getFor(internalFormat);

	DEBUG_ASSERT(formatInfo.isGPUFormat(), "This format can't be loaded on the GPU!");

	bind(0);

	//TODO add back manually generated mipmaps
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	CHECK_GL_ERROR;

	uint32_t destWidth, destHeight;

	//if the platforms supports NPOT, or the dimensions are already POT, direct copy
	if (isPowerOfTwo() || Platform::singleton().isNPOTEnabled()) {
		destWidth = width;
		destHeight = height;
	}
	else {
		destWidth = glm::powerOfTwoAbove(width);
		destHeight = glm::powerOfTwoAbove(height);
	}

	//check if the texture has to be recreated (changed dimensions)
	if (destWidth != internalWidth || destHeight != internalHeight || oldFormat.glFormat != formatInfo.glFormat) {
		internalWidth = destWidth;
		internalHeight = destHeight;
		internalFormat = formatID;
		size = internalWidth * internalHeight * formatInfo.pixelSizeBytes;

		std::string dummyData(size, 0); //needs to preallocate the storage if this tex is used as rendertarget (TODO avoid this if we have data)

		//create an empty GPU mem space
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			formatInfo.glFormat,
			internalWidth,
			internalHeight,
			0,
			formatInfo.glFormat,
			formatInfo.elementType,
			dummyData.c_str());
	}

	UVSize.x = (float)width / (float)internalWidth;
	UVSize.y = (float)height / (float)internalHeight;

	CHECK_GL_ERROR;

	return loaded = true;
}

bool Texture::loadFromMemory(const byte* imageData, int width, int height, PixelFormat sourceFormat, PixelFormat destFormat) {
	DEBUG_ASSERT( imageData, "null image data" );

	loadEmpty(width, height, destFormat);

	auto& format = TexFormatInfo::getFor(sourceFormat);

	mTransparency = false;
	if(destFormat == PixelFormat::R8G8B8A8) {
		auto end = imageData + (width * height * 4);
		for (auto alpha = imageData + 3; alpha < end; alpha += 4) {
			if ( *alpha < 250 ) {
				mTransparency = true;
				break;
			}
		}
	}

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format.glFormat, format.elementType, imageData);
	CHECK_GL_ERROR;

	return loaded = true;
}

bool Texture::loadFromFile(const utf::string& path) {
	DEBUG_ASSERT( !isLoaded(), "The Texture is already loaded" );

	int pixelSize;
	std::vector<byte> imageData;
	auto format = Platform::singleton().loadImageFile(imageData, path, width, height, pixelSize);

	DEBUG_ASSERT_INFO( format != PixelFormat::Unknown, "Cannot load an image file", "path = " + path );

	if (creator.is_some() && creator.unwrap().disableBilinear) {
		disableBilinearFiltering();
	}
	else {
		enableBilinearFiltering();
	}

	bool isSurface = width == glm::powerOfTwoAbove(width) && height == glm::powerOfTwoAbove(height);

	if (!isSurface || (creator.is_some() && creator.unwrap().disableTiling)) {
		disableTiling();
	}
	else {
		enableTiling();
	}

	if (isSurface) { //TODO query anisotropic level
		GLfloat aniso;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
		enableAnisotropicFiltering(aniso / 2);
	}

	loadFromMemory(imageData.data(), width, height, format, PixelFormat::R8G8B8A8);

	return loaded;
}

bool Texture::_setupAtlas() {
	auto& atlas = parentAtlas.unwrap();

	if (!atlas.isLoaded()) {
		return (loaded = false);
	}

	internalWidth = atlas.getInternalWidth();
	internalHeight = atlas.getInternalHeight();

	DEBUG_ASSERT( width > 0 && height > 0 && internalWidth > 0 && internalHeight > 0, "One or more texture dimensions are invalid (less or equal to 0)" );

	//copy bind handle
	glhandle = atlas.glhandle;

	//find uv coordinates
	UVOffset.x = (float)mAtlasOriginX / (float)internalWidth;
	UVOffset.y = (float)mAtlasOriginY / (float)internalHeight;

	//find uv size
	UVSize.x = (float)width / (float)internalWidth;
	UVSize.y = (float)height / (float)internalHeight;

	return (loaded = true);
}

bool Texture::loadFromAtlas(Texture& tex, int x, int y, int sx, int sy) {
	DEBUG_ASSERT( !isLoaded(), "The Texture is already loaded" );

	parentAtlas = tex;
	mTransparency = tex.mTransparency;

	width = sx;
	height = sy;

	mAtlasOriginX = x;
	mAtlasOriginY = y;

	//actual lazy loading is in _setupAtlas

	return false;
}

bool Texture::onLoad() {
	DEBUG_ASSERT( !isLoaded(), "The texture is already loaded" );

	//invalidate the OBB
	OBB.reset();

	if (isReloadable()) {
		return loadFromFile(filePath);
	}
	else if (parentAtlas.is_some()) {
		return _setupAtlas();
	}
	else {
		return false;
	}
}

void Texture::onUnload(bool soft) {
	DEBUG_ASSERT( isLoaded(), "The Texture is not loaded" );

	if (!soft || isReloadable()) {
		if (OBB) {
			OBB->onUnload();
		}

		if (parentAtlas.is_none()) { //don't unload parent texture!
			DEBUG_ASSERT( glhandle, "Tried to unload a texture but the texture handle was invalid" );
			glDeleteTextures(1, &glhandle);

			internalWidth = internalHeight = 0;
			internalFormat = PixelFormat::Unknown;
			glhandle = 0;
			parentAtlas = {};
			mTransparency = false;

			if (mFBO) { //fbos are destroyed on unload, the user must care to rebuild their contents after a purge
				glDeleteFramebuffers(1, &mFBO);
				mFBO = GL_NONE;
			}
		}

		loaded = false;
	}
}


void Texture::_rebuildOptimalBillboard() {
	if (!OBB) {
		OBB = make_unique<Mesh>();

		//build or rebuild the OBB
		OBB->setVertexFields({VertexField::Position2D, VertexField::UV0});
	}

	OBB->begin(4);

	OBB->vertex({-0.5, -0.5});
	OBB->uv(UVOffset.x,
			UVOffset.y + UVSize.y);

	OBB->vertex({0.5, -0.5});
	OBB->uv(UVOffset.x + UVSize.x,
			UVOffset.y + UVSize.y);

	OBB->vertex({-0.5, 0.5});
	OBB->uv(UVOffset.x,
			UVOffset.y);

	OBB->vertex({0.5, 0.5});
	OBB->uv(UVOffset.x + UVSize.x,
			UVOffset.y);

	OBB->end();
}

Mesh& Texture::getOptimalBillboard() {
	if (!OBB) {
		_rebuildOptimalBillboard();
	}

	return *OBB;
}

void Texture::_notifyScreenSize(const Vector& ss) {
	screenSize.x = ss.x;
	screenSize.y = ss.y;
}

void Texture::_notifyOwnerFrameSet(FrameSet& s) {
	DEBUG_ASSERT(ownerFrameSet.is_none(), "Tried to set an owner on an already owned Texture");

	ownerFrameSet = s;
}
