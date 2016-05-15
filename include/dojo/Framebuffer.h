#pragma once

namespace Dojo {
	class Texture;

	class Framebuffer {
	public:
		struct Attachment {
			Texture* texture;
			uint8_t miplevel;
		};

		~Framebuffer();

		void addColorAttachment(Texture& texture, uint8_t miplevel = 0);

		Attachment& getColorAttachment(size_t index) {
			return mColorAttachments[index];
		}

		void enableDepthAttachment() {
			DEBUG_ASSERT(!isCreated(), "Already configured. Too late");
			mHasDepth = true;
		}

		bool hasDepth() const {
			return mHasDepth;
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

		void bind();

	protected:
		std::vector<Attachment> mColorAttachments;
		std::vector<uint32_t> mAttachmentList;
		bool mHasDepth = false;

		uint32_t mFBO = 0, mDepthBuffer = 0;
	private:
	};
}

