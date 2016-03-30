#include "ViewportRecorder.h"

#include "dojo_gl_header.h"
#include "Platform.h"
#include "Renderer.h"
#include "range.h"
#include "Timer.h"
#include "WorkerPool.h"
#include "Viewport.h"
#include "TexFormatInfo.h"
#include "RenderSurface.h"
#include "Texture.h"
#include "Game.h"
#include "Path.h"

using namespace Dojo;
using namespace std::chrono;

ViewportRecorder::ViewportRecorder(Viewport& viewport, Duration videoLength, Duration frequency) :
	mTotalFrameCount(videoLength / frequency),
	mFrequency(frequency) {
	setViewport(viewport);
}

void ViewportRecorder::setViewport(Viewport& viewport) {
	mViewport = viewport;
}

bool ViewportRecorder::hasPBOID(uint32_t ID) const {
	return ID < mInitializedPBOs;
}

ViewportRecorder::~ViewportRecorder() {
	_destroyAllPBOs();
}

void ViewportRecorder::captureFrame() {
	if(mStopRecording) {
		return;
	}

	//TODO get all of these from the currently bound FBO
	//WARNING do not use on OpenGL ES 2!

	auto& surface = mViewport.unwrap().getRenderTarget();
	mFormatInfo = TexFormatInfo::getFor(surface.getFormat());
	mWidth = surface.getWidth();
	mHeight = surface.getHeight();

	auto frameSize = mWidth * mHeight * mFormatInfo.internalPixelSize;
	if (frameSize != mFrameSize) {
		//size changed, throw away all PBOs & recreate them...
		mFrameSize = frameSize;
		_destroyAllPBOs();
	}

	//there are no PBOs, recreate them all
	if(mPBOs.empty()) {
		mPBOs.resize((int)mTotalFrameCount);
		glGenBuffers(mPBOs.size(), mPBOs.data());
		for (auto&& pbo : mPBOs) {
			glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
			glBufferData(GL_PIXEL_PACK_BUFFER, mFrameSize, 0, GL_DYNAMIC_READ);
		}	
		CHECK_GL_ERROR;
	}

	//pick and bind a fresh PBO
	mInitializedPBOs = std::max(mInitializedPBOs, mNextPBO + 1); //mark the current PBO as initialized
	_bindNextPBO();

	if (auto tex = surface.getTexture().to_ref()) {
		tex.get().bindAsRenderTarget(false);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glReadBuffer(GL_BACK);
	}

	glReadPixels(0, 0, surface.getWidth(), surface.getHeight(), mFormatInfo.internalFormat, mFormatInfo.internalElementType, nullptr);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	CHECK_GL_ERROR;
}

void ViewportRecorder::_bindNextPBO() {
	glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBOs[mNextPBO]);
	mNextPBO = (mNextPBO + 1) % mPBOs.size();
}

void ViewportRecorder::_destroyAllPBOs() {
	glDeleteBuffers(mPBOs.size(), mPBOs.data());
	CHECK_GL_ERROR;
	mNextPBO = 0;
	mInitializedPBOs = 0;
	mPBOs.clear();
}

#include <FreeImage.h>
#include <iomanip>

#pragma warning(push)
#pragma warning( disable : 4996 )

std::string getDateString() {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
	return ss.str();
}
#pragma warning(pop)

void ViewportRecorder::makeVideo() {
	mStopRecording = true;
	std::vector<byte*> mappedPointers;
	std::vector<GLuint> mappedPBOs;
	
	//map all the PBOs at the same time and pass them to a background thread
	size_t startFrame = mNextPBO % mInitializedPBOs;

	for (size_t i = 0; i < mInitializedPBOs; ++i) {
		auto idx = (startFrame + i) % mInitializedPBOs;

		glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBOs[idx]);
		auto ptr = (byte*)glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, mFrameSize, GL_MAP_READ_BIT);
		mappedPointers.push_back(ptr);
		mappedPBOs.push_back(mPBOs[idx]);
	}
	CHECK_GL_ERROR;

	Platform::singleton().getBackgroundPool().queue([this, pointers = std::move(mappedPointers)] {
		auto path = Platform::singleton().getPicturesPath();
		path += "/" + Platform::singleton().getGame().getName();
		path += "_" + Path::removeInvalidChars(getDateString());
		path += ".gif";

		FIMULTIBITMAP *multi = FreeImage_OpenMultiBitmap(FIF_GIF, path.bytes().data(), TRUE, FALSE);

		DWORD dwFrameTime = (DWORD)duration_cast<milliseconds>(mFrequency).count();

		for (auto&& data : pointers) {
			//swap red and blue
			for (uint32_t i = 0; i < mFrameSize; i += mFormatInfo.internalPixelSize) {
				std::swap(data[i], data[i + 2]);
			}

			auto dibHiDef = FreeImage_ConvertFromRawBits(
				data,
				mWidth,
				mHeight,
				mWidth * mFormatInfo.internalPixelSize,
				mFormatInfo.internalPixelSize * 8,
				FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
				true);

			auto dib = FreeImage_ColorQuantize(dibHiDef, FIQ_WUQUANT);
			FreeImage_Unload(dibHiDef);

			// clear any animation metadata used by this dib as we’ll adding our own ones
			FreeImage_SetMetadata(FIMD_ANIMATION, dib, NULL, NULL);
			// add animation tags to dib  
			FITAG *tag = FreeImage_CreateTag();
			if (tag) {
				FreeImage_SetTagKey(tag, "FrameTime");
				FreeImage_SetTagType(tag, FIDT_LONG);
				FreeImage_SetTagCount(tag, 1);
				FreeImage_SetTagLength(tag, 4);
				FreeImage_SetTagValue(tag, &dwFrameTime);
				FreeImage_SetMetadata(FIMD_ANIMATION, dib, FreeImage_GetTagKey(tag), tag);
				FreeImage_DeleteTag(tag);
			}
			FreeImage_AppendPage(multi, dib);
			FreeImage_Unload(dib);
		}
		FreeImage_CloseMultiBitmap(multi);
	},
	[this, pbos = std::move(mappedPBOs)] {
		//finally, unmap the buffers
		for(auto&& pbo : pbos) {
			glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		}
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		CHECK_GL_ERROR;

		mStopRecording = false; //resume recording
	});
}
