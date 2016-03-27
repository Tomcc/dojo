#pragma once

#include "dojo_common_header.h"

#include "FrameSubmitter.h"

namespace Dojo {
	//a class that just before submitting a frame, it adds it to a recorded stream
	class RecorderSubmitter : public FrameSubmitter {
	public:
		RecorderSubmitter(FrameSubmitter& baseSubmitter, uint32_t FPS, uint32_t ringBufferSize);
		virtual ~RecorderSubmitter();

		void setFPS(uint32_t FPS);

		bool hasPBOID(uint32_t ID) const;

		virtual void submitFrame() override;
	protected:
		std::chrono::high_resolution_clock::duration mCaptureInterval;
		std::chrono::high_resolution_clock::time_point mNextCaptureTime;
		FrameSubmitter& mBase;
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

