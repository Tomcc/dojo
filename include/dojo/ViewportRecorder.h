#pragma once

#include "dojo_common_header.h"

#include "TimedEvent.h"

namespace Dojo {
	class Viewport;

	//a class that just before submitting a frame, it adds it to a recorded stream
	class ViewportRecorder {
	public:
		ViewportRecorder(Viewport& viewport);
		virtual ~ViewportRecorder();

		void setViewport(Viewport& viewport);

		bool hasPBOID(uint32_t ID) const;

		void captureFrame();
	protected:
		optional_ref<Viewport> mViewport;
		uint32_t mFrameSize = 0;
		std::vector<Shared<std::vector<byte>>> mBuffers;

		std::vector<uint32_t> mPBOs;
		size_t mNextPBO = 0;
		size_t mMaxPBOCount;
		std::atomic<bool> mLockedReading;
		uint32_t mLockedPBO = 0;
		
		void _bindNextPBO();

		void _startFrameCapture();
		void _writePBO(uint32_t PBO);
		void _destroyAllPBOs();

		Shared<std::vector<byte>> _getBuffer(uint32_t size);
	};
}

