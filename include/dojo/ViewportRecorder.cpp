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

ViewportRecorder::ViewportRecorder(Viewport& viewport) :
	mMaxPBOCount(3),
	mLockedReading(false) {
	setViewport(viewport);
}

void ViewportRecorder::setViewport(Viewport& viewport) {
	mViewport = viewport;
}

bool ViewportRecorder::hasPBOID(uint32_t ID) const {
	return ID < mPBOs.size();
}

Shared<std::vector<byte>> ViewportRecorder::_getBuffer(uint32_t size) {
	if(mBuffers.size() > 0) {
		auto buf = std::move(mBuffers.back());
		mBuffers.pop_back();
		buf->resize(size);
		return buf;
	}
	return make_shared<std::vector<byte>>(size); //return a new one. It'll come back
}

ViewportRecorder::~ViewportRecorder() {
	_destroyAllPBOs();
}

void ViewportRecorder::captureFrame() {
	//check if it's done reading in the background and unmap the buffer
	if(mLockedReading == false && mLockedPBO) {
		glBindBuffer(GL_PIXEL_PACK_BUFFER, mLockedPBO);
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		mLockedPBO = 0;
	}

	_startFrameCapture();
}

void ViewportRecorder::_bindNextPBO() {
	if (hasPBOID(mNextPBO)) {
		glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBOs[mNextPBO]);
	}
	else {
		GLuint pbo;
		glGenBuffers(1, &pbo);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
		glBufferData(GL_PIXEL_PACK_BUFFER, mFrameSize, 0, GL_DYNAMIC_READ);
		CHECK_GL_ERROR;
		mPBOs.emplace_back(pbo);
	}

	mNextPBO = (mNextPBO + 1) % mMaxPBOCount;
}

void ViewportRecorder::_destroyAllPBOs() {
	DEBUG_ASSERT(!mLockedReading, "Cannot delete now");

	glDeleteBuffers(mPBOs.size(), mPBOs.data());
	CHECK_GL_ERROR;
	mNextPBO = 0;
}

void ViewportRecorder::_writePBO(uint32_t PBO) {
	mLockedReading = true;
	mLockedPBO = PBO;
	auto size = mFrameSize; //copy the size because it might change before the task runs

	glBindBuffer(GL_PIXEL_PACK_BUFFER, PBO);
	auto ptr = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, mFrameSize, GL_MAP_READ_BIT);
 	auto bufptr = _getBuffer(mFrameSize);

	//send the buffer to another thread to compress it
	Platform::singleton().getBackgroundPool().queue([this, buf = bufptr, ptr, size] {
		memcpy(buf->data(), ptr, size);
		mLockedReading = false;

		//TODO downscale, compress and store
	},
	[this, bufptr, PBO] {
		//put back the buffer
		mBuffers.emplace_back(bufptr);
	});
}

void ViewportRecorder::_startFrameCapture() {
	//TODO get all of these from the currently bound FBO
	//WARNING do not use on OpenGL ES 2!

	auto& surface = mViewport.unwrap().getRenderTarget();
	auto desc = TexFormatInfo::getFor(surface.getFormat());

	DEBUG_ASSERT(mLockedReading == false, "Reading the next buffer should be done by now as now it needs to be reused");

	auto frameSize = surface.getWidth() * surface.getHeight() * desc.pixelSizeBytes;
	if(frameSize != mFrameSize) {
		//size changed, throw away all PBOs & recreate them...
		mFrameSize = frameSize;
		_destroyAllPBOs();
	}

	//pick and bind a fresh PBO
	_bindNextPBO();

	if(auto tex = surface.getTexture().to_ref()) {
		tex.get().bindAsRenderTarget(false);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glReadBuffer(GL_BACK);
	}
	
	glReadPixels(0, 0, surface.getWidth(), surface.getHeight(), desc.glFormat, desc.elementType, nullptr);
	CHECK_GL_ERROR;

	//also, if there is a "next PBO" initialized it's time to write it down
	//as next frame it will be used
	if(hasPBOID(mNextPBO)) {
		_writePBO(mPBOs[mNextPBO]);
	}

	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	CHECK_GL_ERROR;
}
