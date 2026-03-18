#pragma once

#include "Engine/Renderer/RendererAPI.h"

namespace GameEngine {

	class RenderCommand
	{
	public:
		static void Init()
		{
			gsRendererAPI->Init();
		}

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			gsRendererAPI->SetViewport(x, y, width, height);
		}

		static void SetClearColor(const glm::vec4& color)
		{
			gsRendererAPI->SetClearColor(color);
		}

		static void Clear()
		{
			gsRendererAPI->Clear();
		}

		static void DrawIndexed(const Handle<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			gsRendererAPI->DrawIndexed(vertexArray, indexCount);
		}

		static void DrawLines(const Handle<VertexArray>& vertexArray, uint32_t vertexCount)
		{
			gsRendererAPI->DrawLines(vertexArray, vertexCount);
		}

		static void SetLineWidth(float width)
		{
			gsRendererAPI->SetLineWidth(width);
		}
	private:
		static Own<RendererAPI> gsRendererAPI;
	};

}
