#include "Framebuffer.h"

#include "dojo_gl_header.h"

#include "Texture.h"
#include "Platform.h"
#include "Renderer.h"

using namespace Dojo;

void Framebuffer::addColorAttachment(Texture& texture, uint8_t miplevel /*= 0*/) {
	DEBUG_ASSERT(!isCreated(), "Already configured. Too late");
	mColorAttachments.emplace_back(Attachment{ &texture, miplevel });
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
	}
	else if (!isCreated()) {
		//create the framebuffer and attach all the stuff

		glGenFramebuffers(1, &mFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

		CHECK_GL_ERROR;

		auto width = getWidth();
		auto height = getHeight();
		int i = 0;
		for(auto&& color : mColorAttachments) {
			color.texture->_addAsAttachment(i, width, height, color.miplevel);
		}

		if (mHasDepth) {
			glGenRenderbuffersEXT(1, &mDepthBuffer);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mDepthBuffer);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT16, width, height);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, mDepthBuffer);

			CHECK_GL_ERROR;
		}

		auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		DEBUG_ASSERT(status == GL_FRAMEBUFFER_COMPLETE, "The framebuffer is incomplete");
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
		glFrontFace(GL_CW); //invert vertex winding when inverting the view
	}

	glReadBuffer(mColorAttachments.empty() ? GL_BACK : GL_COLOR_ATTACHMENT0);
}

Dojo::Framebuffer::~Framebuffer() {
	if (isCreated()) { //fbos are destroyed on unload, the user must care to rebuild their contents after a purge
		glDeleteFramebuffers(1, &mFBO);
	}
}
