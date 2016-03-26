#include "RecorderSubmitter.h"

#include "dojo_gl_header.h"
#include "Platform.h"
#include "Renderer.h"
#include "range.h"
#include "Timer.h"

using namespace Dojo;
using namespace std::chrono;

Dojo::RecorderSubmitter::RecorderSubmitter(FrameSubmitter& baseSubmitter, uint32_t FPS) :
	mBase(baseSubmitter) {
	setFPS(FPS);

	mNextCaptureTime = high_resolution_clock::now();
}

RecorderSubmitter::~RecorderSubmitter() {
	glDeleteBuffers(2, mPBO);
}

void RecorderSubmitter::setFPS(uint32_t FPS) {
	auto invFPS = duration<float>(1.f / FPS);
	mCaptureInterval = duration_cast<high_resolution_clock::duration>(invFPS);
}

void RecorderSubmitter::submitFrame() {
	auto now = high_resolution_clock::now();
	if(now >= mNextCaptureTime) {
		_startFrameCapture();
		mNextCaptureTime = now + mCaptureInterval;
	}

	mBase.submitFrame();
}

void RecorderSubmitter::_writePBO(uint32_t PBO) {
	//TODO
}

void RecorderSubmitter::_startFrameCapture() {
	//TODO get all of these from the currently bound FBO
	auto w = Platform::singleton().getScreenWidth();
	auto h = Platform::singleton().getScreenHeight();
	uint32_t pixelSize = 4;
	auto format = GL_RGBA;
	auto channelFormat = GL_UNSIGNED_BYTE;

	auto frameSize = w * h * pixelSize;
	if(frameSize != mFrameSize) {
		//size changed, throw away all PBOs & recreate them...
		mFrameSize = frameSize;
		mPBO[0] = mPBO[1] = 0;
	}

	//if the buffer that needs to be written to is 0, then generate it
	auto& pbo = mPBO[0];
	if(pbo == 0) {
		glGenBuffers(1, &pbo);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
		glBufferData(GL_PIXEL_PACK_BUFFER, mFrameSize, 0, GL_DYNAMIC_READ);
		CHECK_GL_ERROR;
	}
	else { //else write it out so that it can be rewritten
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
		_writePBO(pbo);
	}

	//pick and bind a fresh PBO
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, w, h, format, channelFormat, nullptr);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	//swap the buffers
	std::swap(mPBO[0], mPBO[1]);

	CHECK_GL_ERROR;
}
