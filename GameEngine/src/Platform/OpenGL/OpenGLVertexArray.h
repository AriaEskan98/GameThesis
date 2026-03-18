#pragma once

#include "Engine/Renderer/VertexArray.h"

namespace GameEngine {

	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const Handle<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const Handle<IndexBuffer>& indexBuffer) override;

		virtual const std::vector<Handle<VertexBuffer>>& GetVertexBuffers() const { return myVertexBuffers; }
		virtual const Handle<IndexBuffer>& GetIndexBuffer() const { return myIndexBuffer; }
	private:
		uint32_t myRendererID;
		uint32_t myVertexBufferIndex = 0;
		std::vector<Handle<VertexBuffer>> myVertexBuffers;
		Handle<IndexBuffer> myIndexBuffer;
	};

}
