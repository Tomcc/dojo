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

using namespace Dojo;
using namespace std::chrono;

ViewportRecorder::ViewportRecorder(Viewport& viewport, Duration videoLength, Duration frequency) :
	mTotalFrameCount(videoLength / frequency) {
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
	//TODO get all of these from the currently bound FBO
	//WARNING do not use on OpenGL ES 2!

	auto& surface = mViewport.unwrap().getRenderTarget();
	auto desc = TexFormatInfo::getFor(surface.getFormat());

	auto frameSize = surface.getWidth() * surface.getHeight() * desc.pixelSizeBytes;
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
	_bindNextPBO();
	mInitializedPBOs = std::max(mInitializedPBOs, mNextPBO); //mark the current PBO as initialized

	if (auto tex = surface.getTexture().to_ref()) {
		tex.get().bindAsRenderTarget(false);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glReadBuffer(GL_BACK);
	}

	glReadPixels(0, 0, surface.getWidth(), surface.getHeight(), desc.glFormat, desc.elementType, nullptr);
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
