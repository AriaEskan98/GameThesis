#pragma once

#include "Engine/Renderer/Framebuffer.h"

namespace GameEngine {

	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer();

		void Invalidate();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override { GE_CORE_ASSERT(index < myColorAttachments.size()); return myColorAttachments[index]; }

		virtual const FramebufferSpecification& GetSpecification() const override { return mySpecification; }
	private:
		uint32_t myRendererID = 0;
		FramebufferSpecification mySpecification;

		std::vector<FramebufferTextureSpecification> myColorAttachmentSpecifications;
		FramebufferTextureSpecification myDepthAttachmentSpecification = FramebufferTextureFormat::None;

		std::vector<uint32_t> myColorAttachments;
		uint32_t myDepthAttachment = 0;
	};

}
