#pragma once

#include "dojo_common_header.h"

#include "FrameSubmitter.h"

namespace Dojo {
	//a class that just before submitting a frame, it adds it to a recorded stream
	class RecorderSubmitter : public FrameSubmitter {
	public:
		RecorderSubmitter(FrameSubmitter& baseSubmitter, uint32_t FPS);
		virtual ~RecorderSubmitter();

		void setFPS(uint32_t FPS);

		virtual void submitFrame() override;

	protected:
		std::chrono::high_resolution_clock::duration mCaptureInterval;
		std::chrono::high_resolution_clock::time_point mNextCaptureTime;
		FrameSubmitter& mBase;
		uint32_t mFrameSize = 0;

		uint32_t mPBO[2];
		
		uint32_t _bindNextPBO();

		void _startFrameCapture();
		void _writePBO(uint32_t PBO);
	};
}

