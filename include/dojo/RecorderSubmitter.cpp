#include "RecorderSubmitter.h"

#include "dojo_gl_header.h"
#include "Platform.h"
#include "Renderer.h"
#include "range.h"
#include "Timer.h"
#include "WorkerPool.h"

#pragma warning(push, 0)  
extern "C" {
	#include <libswscale/swscale.h>
}
#pragma warning(pop)

using namespace Dojo;
using namespace std::chrono;

Dojo::RecorderSubmitter::RecorderSubmitter(FrameSubmitter& baseSubmitter, uint32_t FPS, uint32_t ringBufferSize) :
	mBase(baseSubmitter),
	mMaxPBOCount(ringBufferSize) {

	setFPS(FPS);
	mNextCaptureTime = high_resolution_clock::now();
	mLockedReading = false;
}

bool Dojo::RecorderSubmitter::hasPBOID(uint32_t ID) const {
	return ID < mPBOs.size();
}

Shared<std::vector<byte>> RecorderSubmitter::_getBuffer(uint32_t size) {
	if(mBuffers.size() > 0) {
		auto buf = std::move(mBuffers.back());
		mBuffers.pop_back();
		buf->resize(size);
		return buf;
	}
	return make_shared<std::vector<byte>>(size); //return a new one. It'll come back
}

RecorderSubmitter::~RecorderSubmitter() {
	_destroyAllPBOs();
}

void RecorderSubmitter::setFPS(uint32_t FPS) {
	auto invFPS = duration<float>(1.f / FPS);
	mCaptureInterval = duration_cast<high_resolution_clock::duration>(invFPS);
}

void RecorderSubmitter::submitFrame() {

	//check if it's done reading in the background and unmap the buffer
	if(mLockedReading == false && mLockedPBO) {
		glBindBuffer(GL_PIXEL_PACK_BUFFER, mLockedPBO);
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		mLockedPBO = 0;
	}

	auto now = high_resolution_clock::now();
	if(now >= mNextCaptureTime) {
		_startFrameCapture();
		mNextCaptureTime = now + mCaptureInterval;
	}

	mBase.submitFrame();
}

void RecorderSubmitter::_bindNextPBO() {
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

void Dojo::RecorderSubmitter::_destroyAllPBOs() {
	DEBUG_ASSERT(!mLockedReading, "Cannot delete now");

	glDeleteBuffers(mPBOs.size(), mPBOs.data());
	CHECK_GL_ERROR;
	mNextPBO = 0;
}

void RecorderSubmitter::_writePBO(uint32_t PBO) {
	mLockedReading = true;
	mLockedPBO = PBO;
	auto size = mFrameSize; //copy the size because it might change before the task runs

	glBindBuffer(GL_PIXEL_PACK_BUFFER, PBO);
	auto ptr = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, mFrameSize, GL_MAP_READ_BIT);
 	auto bufptr = _getBuffer(mFrameSize);

	//send the buffer to another thread to compress it
	Platform::singleton().getWorkerPool().queue([this, buf = bufptr, ptr, size] {
		memcpy(buf->data(), ptr, size);
		mLockedReading = false;

		//TODO downscale, compress and store
	},
	[this, bufptr, PBO] {
		//put back the buffer
		mBuffers.emplace_back(bufptr);
	});
}

void RecorderSubmitter::_startFrameCapture() {
	//TODO get all of these from the currently bound FBO
	//WARNING do not use on OpenGL ES 2!
	auto w = Platform::singleton().getScreenWidth();
	auto h = Platform::singleton().getScreenHeight();
	uint32_t pixelSize = 4;
	auto format = GL_RGBA;
	auto channelFormat = GL_UNSIGNED_BYTE;

	DEBUG_ASSERT(mLockedReading == false, "Reading the next buffer should be done by now as now it needs to be reused");

	auto frameSize = w * h * pixelSize;
	if(frameSize != mFrameSize) {
		//size changed, throw away all PBOs & recreate them...
		mFrameSize = frameSize;
		_destroyAllPBOs();
	}

	_bindNextPBO();

	//pick and bind a fresh PBO
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, w, h, format, channelFormat, nullptr);
	CHECK_GL_ERROR;

	//also, if there is a "next PBO" initialized it's time to write it down
	//as next frame it will be used
	if(hasPBOID(mNextPBO)) {
		_writePBO(mPBOs[mNextPBO]);
	}

	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	CHECK_GL_ERROR;

}
