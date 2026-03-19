#include "gepch.h"
#include "GameEngine/Renderer/Renderer.h"
#include "GameEngine/Renderer/Renderer3D.h"

namespace GameEngine {

	Own<Renderer::SceneData> Renderer::gsSceneData = MakeOwn<Renderer::SceneData>();

	void Renderer::Init()
	{
		GE_PROFILE_FUNCTION();

		RenderCommand::Init();
		Renderer3D::Init();
	}

	void Renderer::Shutdown()
	{
		Renderer3D::Shutdown();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		gsSceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Submit(const Handle<Shader>& shader, const Handle<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		shader->Bind();
		shader->SetMat4("u_ViewProjection", gsSceneData->ViewProjectionMatrix);
		shader->SetMat4("u_Transform", transform);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

}
