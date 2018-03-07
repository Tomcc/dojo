#include "Framebuffer.h"

#include <glad/glad.h>

#include "Texture.h"
#include "Platform.h"
#include "Renderer.h"

namespace Dojo {
	class RenderBuffer {
	public:
		RenderBuffer() {}

		void _initAndBind(GLenum pixelFormat, uint32_t width, uint32_t height) {
			DEBUG_ASSERT(not isInited(), "Already initialized");
			DEBUG_ASSERT(width > 0 and height > 0, "Invalid dimensions");

			mWidth = width;
			mHeight = height;

			glGenRenderbuffers(1, &handle);
			glBindRenderbuffer(GL_RENDERBUFFER, handle);
			glRenderbufferStorage(GL_RENDERBUFFER, pixelFormat, width, height);
		}

		~RenderBuffer() {
			if (handle) {
				glDeleteRenderbuffers(1, &handle);
			}
		}

		bool isInited() const {
			return handle != 0;
		}

		GLuint handle = 0;
		uint32_t mWidth = 0, mHeight = 0;
	};

	Dojo::Framebuffer::~Framebuffer() {
		if (isCreated()) { //fbos are destroyed on unload, the user must care to rebuild their contents after a purge
			glDeleteFramebuffers(1, &mFBO);
		}
	}

	void Framebuffer::addColorAttachment(Texture& texture, uint8_t miplevel /*= 0*/) {
		DEBUG_ASSERT(!isCreated(), "Already configured. Too late");
		mColorAttachments.emplace_back(TextureAttachment{ &texture, miplevel });
	}

	void Framebuffer::addDepthAttachment(std::shared_ptr<RenderBuffer> buffer /*= nullptr*/) {
		DEBUG_ASSERT(!isCreated(), "Already configured. Too late");
		if (buffer) {
			mDepthBuffer = std::move(buffer);
		}
		else {
			mDepthBuffer = make_shared<RenderBuffer>();
		}
	}

	uint32_t Framebuffer::getWidth() const {
		if (isBackbuffer()) {
			return Platform::singleton().getRenderer().getBackbuffer().getWidth();
		}
		else {
			auto& attach = mColorAttachments[0];
			return attach.texture->getWidth() >> attach.miplevel;
		}
	}

	uint32_t Framebuffer::getHeight() const {
		if (isBackbuffer()) {
			return Platform::singleton().getRenderer().getBackbuffer().getHeight();
		}
		else {
			auto& attach = mColorAttachments[0];
			return attach.texture->getHeight() >> attach.miplevel;
		}
	}

	void Framebuffer::bind() {

		if (isBackbuffer()) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glFrontFace(GL_CCW);
			glReadBuffer(GL_BACK);
			GLenum buffer[] = { GL_BACK };
			glDrawBuffers(1, buffer);
		}
		else {
			if (not isCreated()) {
				//create the framebuffer and attach all the stuff

				glGenFramebuffers(1, &mFBO);
				glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

				auto width = getWidth();
				auto height = getHeight();
				uint32_t i = 0;
				for (auto&& color : mColorAttachments) {
					color.texture->_addAsAttachment(i, width, height, color.miplevel);
					mAttachmentList.push_back(GL_COLOR_ATTACHMENT0 + i);
					++i;
				}

				if (mDepthBuffer) {
					if (mDepthBuffer->isInited()) {
						DEBUG_ASSERT(mDepthBuffer->mWidth = width and mDepthBuffer->mHeight == height, "Mismatched dimensions");
						glBindRenderbuffer(GL_RENDERBUFFER, mDepthBuffer->handle);
					}
					else {
						//create one
						mDepthBuffer->_initAndBind(GL_DEPTH_COMPONENT16, width, height);
					}
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBuffer->handle);
					mAttachmentList.push_back(GL_DEPTH_ATTACHMENT);
				}

				auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
				DEBUG_ASSERT(status == GL_FRAMEBUFFER_COMPLETE, "The framebuffer is incomplete");
			}
			else {
				glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
			}

			glFrontFace(GL_CW); //invert vertex winding when inverting the view
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glDrawBuffers(static_cast<GLsizei>(mAttachmentList.size() - hasDepth()), mAttachmentList.data());
		}
	}

	void Framebuffer::invalidate() {
		if (not isBackbuffer()) {
			bind();

			glInvalidateFramebuffer(GL_FRAMEBUFFER, static_cast<GLsizei>(mAttachmentList.size()), mAttachmentList.data());
		}
	}
}