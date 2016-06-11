#pragma once

#include "dojo_common_header.h"

namespace Dojo {
	class Texture;
	class RenderBuffer;

	class Framebuffer {
	public:
		struct TextureAttachment {
			Texture* texture;
			uint8_t miplevel;
		};

		~Framebuffer();

		void addColorAttachment(Texture& texture, uint8_t miplevel = 0);

		///enable depth on this framebuffer. Will use the provided buffer if any
		void addDepthAttachment(std::shared_ptr<RenderBuffer> buffer = nullptr);

		TextureAttachment& getColorAttachment(size_t index) {
			return mColorAttachments[index];
		}
		
		bool hasDepth() const {
			return mDepthBuffer != nullptr;
		}

		bool isCreated() const {
			return mFBO != 0;
		}

		uint32_t getWidth() const;
		uint32_t getHeight() const;

		bool isBackbuffer() const {
			return mColorAttachments.empty();
		}

		bool isFlipped() const {
			return not isBackbuffer();
		}

		void invalidate();

		void bind();

		std::weak_ptr<RenderBuffer> getDepthBuffer() {
			return mDepthBuffer;
		}

	protected:
		std::vector<TextureAttachment> mColorAttachments;
		std::shared_ptr<RenderBuffer> mDepthBuffer;

		std::vector<uint32_t> mAttachmentList;
		
		uint32_t mFBO = 0;

	private:
	};
}

