#include "gepch.h"
#include "Hazel/Renderer/Renderer3D.h"

#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/UniformBuffer.h"

#include <glm/gtc/matrix_transform.hpp>

namespace GameEngine {

	Own<Renderer3D::SceneData> Renderer3D::gsSceneData = MakeOwn<Renderer3D::SceneData>();
	Handle<Shader>             Renderer3D::gsMeshShader;

	void Renderer3D::Init()
	{
		GE_PROFILE_FUNCTION();

		gsMeshShader = Shader::Create("assets/shaders/Mesh.glsl");
	}

	void Renderer3D::Shutdown()
	{
		gsMeshShader.reset();
	}

	void Renderer3D::BeginScene(const EditorCamera& camera)
	{
		gsSceneData->ViewProjection = camera.GetViewProjection();
	}

	void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& cameraTransform)
	{
		gsSceneData->ViewProjection = camera.GetProjection() * glm::inverse(cameraTransform);
	}

	void Renderer3D::EndScene()
	{
	}

	void Renderer3D::Submit(const Handle<Mesh>& mesh, const glm::mat4& transform,
	                        const glm::vec4& color, int entityID)
	{
		if (!mesh)
			return;

		gsMeshShader->Bind();
		gsMeshShader->SetMat4("u_ViewProjection", gsSceneData->ViewProjection);
		gsMeshShader->SetMat4("u_Transform",      transform);
		gsMeshShader->SetFloat4("u_Color",        color);
		gsMeshShader->SetInt("u_EntityID",        entityID);

		RenderCommand::DrawIndexed(mesh->GetVertexArray(), mesh->GetIndexCount());
	}

}
