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
	glhandle(0) {
}

Texture::Texture(optional_ref<ResourceGroup> creator, const utf::string& path) :
	Resource(creator, path),
	internalWidth(0),
	internalHeight(0),
	glhandle(0) {
}

Texture::~Texture() {
	OBB.reset();

	if (loaded) {
		onUnload();
	}
}

void Texture::bind(uint32_t index) {
	//create the gl texture if still not created!
	DEBUG_ASSERT(glhandle, "This texture wasn't created yet");

	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, glhandle);
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

void Dojo::Texture::_addAsAttachment(uint32_t index, uint32_t width, uint32_t height, uint8_t miplevel) {
	DEBUG_ASSERT(width == getWidth() and height == getHeight(), "Cannot add texture as attachment");
	DEBUG_ASSERT(miplevel == 0, "Mipmaps aren't supported anymore :(");

	bind(0);

	//TODO use the proper types
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0 + index,
		GL_TEXTURE_2D,
		glhandle,
		miplevel
	);

}

bool Dojo::Texture::_createStorage(uint32_t w, uint32_t h, PixelFormat formatID) {
	width = w;
	height = h;

	DEBUG_ASSERT(width > 0, "Width must be more than 0");
	DEBUG_ASSERT(height > 0, "Height must be more than 0");

	auto& formatInfo = TexFormatInfo::getFor(formatID);
	auto& oldFormat = TexFormatInfo::getFor(internalFormat);

	DEBUG_ASSERT(formatInfo.isGPUFormat(), "This format can't be loaded on the GPU!");

	if (not glhandle) {
		glGenTextures(1, &glhandle);
	}
	glBindTexture(GL_TEXTURE_2D, glhandle);

	//TODO add back manually generated mipmaps
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	uint32_t destWidth, destHeight;

	//if the platforms supports NPOT, or the dimensions are already POT, direct copy
	if (isPowerOfTwo() or Platform::singleton().isNPOTEnabled()) {
		destWidth = width;
		destHeight = height;
	}
	else {
		destWidth = glm::ceilPowerOfTwo(width);
		destHeight = glm::ceilPowerOfTwo(height);
	}

	//check if the texture has to be recreated (changed dimensions)
	if (destWidth != internalWidth or destHeight != internalHeight or oldFormat.internalFormat != formatInfo.internalFormat) {
		internalWidth = destWidth;
		internalHeight = destHeight;
		internalFormat = formatID;

		auto internalSize = internalWidth * internalHeight * formatInfo.sourcePixelSize;
		DEBUG_ASSERT(internalSize % 4 == 0, "OpenGL implementations choke on non-4-aligned buffers");

		glTexStorage2D(
			GL_TEXTURE_2D,
			1, //TODO multiple levels?
			formatInfo.internalFormat,
			internalWidth,
			internalHeight
		);


	}

	UVSize.x = (float)width / (float)internalWidth;
	UVSize.y = (float)height / (float)internalHeight;

	return loaded = true;
}

bool Dojo::Texture::loadEmpty(uint32_t width, uint32_t height, PixelFormat destFormat) {
	return _createStorage(width, height, destFormat);
}

const uint8_t* convertToGPUFormat(const uint8_t* imageData, uint32_t width, uint32_t height, PixelFormat& inoutFormat, std::vector<uint8_t>& backingBuffer) {
	if (inoutFormat == PixelFormat::RGBA_8_8_8_8 or inoutFormat == PixelFormat::RGBA_8_8_8_8_SRGB) {
		return imageData;
	}
	else if(inoutFormat == PixelFormat::RGB_8_8_8 or inoutFormat == PixelFormat::RGB_8_8_8_SRGB) {
		if(inoutFormat == PixelFormat::RGB_8_8_8) {
			inoutFormat = PixelFormat::RGBA_8_8_8_8;
		}
		else if (inoutFormat == PixelFormat::RGB_8_8_8_SRGB) {
			inoutFormat = PixelFormat::RGBA_8_8_8_8_SRGB;
		}

		//now allocate enough memory and add alpha
		backingBuffer.resize(width * height * 4, 255);
		auto in = imageData;
		auto end = imageData + width * height * 3;
		auto out = backingBuffer.data();
		while (in < end) {
			out[0] += in[0];
			out[1] += in[1];
			out[2] += in[2];
			in += 3;
			out += 4;
		}
		return backingBuffer.data();
	}
	else {
		FAIL("Format not supported for now...");
	}
}

bool Texture::loadFromMemory(const uint8_t* imageData, uint32_t width, uint32_t height, PixelFormat format) {
	DEBUG_ASSERT(imageData, "null image data");
	DEBUG_ASSERT(width > 0 and height > 0, "Invalid dimensions");

	//if needed, convert the format and the data to something the GPU supports
	std::vector<uint8_t> conversionBuffer;
	imageData = convertToGPUFormat(imageData, width, height, format, conversionBuffer);

	_createStorage(width, height, format);

	auto& formatDesc = TexFormatInfo::getFor(format);

	mTransparency = false;
	if (formatDesc.hasAlpha) {
		auto end = imageData + (width * height * 4);
		for (auto alpha = imageData + 3; alpha < end; alpha += 4) {
			if (*alpha < 250) {
				mTransparency = true;
				break;
			}
		}
	}

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, formatDesc.sourceFormat, formatDesc.sourceElementType, imageData);

	return loaded = true;
}

bool Texture::loadFromFile(const utf::string& path) {
	DEBUG_ASSERT(not isLoaded(), "The Texture is already loaded");

	if (not glhandle) {
		glGenTextures(1, &glhandle);
	}

	int pixelSize;
	std::vector<uint8_t> imageData;
	auto format = Platform::singleton().loadImageFile(imageData, path, width, height, pixelSize);

	DEBUG_ASSERT_INFO(format != PixelFormat::Unknown, "Cannot load an image file", "path = " + path);

	if (creator.is_some() and creator.unwrap().disableBilinear) {
		disableBilinearFiltering();
	}
	else {
		enableBilinearFiltering();
	}

	enableTiling();

	loadFromMemory(imageData.data(), width, height, format);

	return loaded;
}

bool Texture::_setupAtlas() {
	auto& atlas = parentAtlas.unwrap();

	if (not atlas.isLoaded()) {
		return (loaded = false);
	}

	internalWidth = atlas.getInternalWidth();
	internalHeight = atlas.getInternalHeight();

	DEBUG_ASSERT(width > 0 and height > 0 and internalWidth > 0 and internalHeight > 0, "One or more texture dimensions are invalid (less or equal to 0)");

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
	DEBUG_ASSERT(not isLoaded(), "The Texture is already loaded");

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
	DEBUG_ASSERT(not isLoaded(), "The texture is already loaded");

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
	DEBUG_ASSERT(isLoaded(), "The Texture is not loaded");

	if (not soft or isReloadable()) {
		if (OBB) {
			OBB->onUnload();
		}

		if (parentAtlas.is_none()) { //don't unload parent texture!
			DEBUG_ASSERT(glhandle, "Tried to unload a texture but the texture handle was invalid");
			glDeleteTextures(1, &glhandle);

			internalWidth = internalHeight = 0;
			internalFormat = PixelFormat::Unknown;
			glhandle = 0;
			parentAtlas = {};
			mTransparency = false;
		}

		loaded = false;
	}
}


void Texture::_rebuildOptimalBillboard() {
	if (not OBB) {
		OBB = make_unique<Mesh>();

		//build or rebuild the OBB
		OBB->setVertexFields({ VertexField::Position2D, VertexField::UV0 });
	}

	OBB->begin(4);

	OBB->vertex({ -0.5, -0.5 });
	OBB->uv(UVOffset.x,
		UVOffset.y + UVSize.y);

	OBB->vertex({ 0.5, -0.5 });
	OBB->uv(UVOffset.x + UVSize.x,
		UVOffset.y + UVSize.y);

	OBB->vertex({ -0.5, 0.5 });
	OBB->uv(UVOffset.x,
		UVOffset.y);

	OBB->vertex({ 0.5, 0.5 });
	OBB->uv(UVOffset.x + UVSize.x,
		UVOffset.y);

	OBB->end();
}

Mesh& Texture::getOptimalBillboard() {
	if (not OBB) {
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
