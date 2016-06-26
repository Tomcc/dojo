#pragma once

#include "dojo_common_header.h"

#include "TimedEvent.h"
#include "TexFormatInfo.h"
#include "AsyncJob.h"

namespace Dojo {
	class Viewport;

	//a class that just before submitting a frame, it adds it to a recorded stream
	class ViewportRecorder {
	public:
		ViewportRecorder(Viewport& viewport, Duration videoLength, Duration frequency);
		virtual ~ViewportRecorder();

		void setViewport(Viewport& viewport);

		bool hasPBOID(uint32_t ID) const;

		void captureFrame();

		void makeVideo();

	private:
		optional_ref<Viewport> mViewport;

		uint32_t mFrameSize = 0;
		uint32_t mWidth, mHeight;
		TexFormatInfo mFormatInfo;

		std::vector<uint32_t> mPBOs;
		size_t mInitializedPBOs = 0;
		size_t mNextPBO = 0;
		int64_t mTotalFrameCount;
		Duration mFrequency;

		AsyncJob::StatusPtr mAsyncJobStatus;
		
		void _bindNextPBO();
		void _destroyAllPBOs();

	};
}

