#include "gepch.h"
#include "GameEngine/Renderer/Renderer3D.h"

#include "GameEngine/Core/Application.h"
#include "GameEngine/Renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace GameEngine {

	Own<Renderer3D::SceneData> Renderer3D::gsData;
	Handle<Shader>             Renderer3D::gsMeshShader;
	Handle<Texture2D>          Renderer3D::gsDefaultTexture;

	void Renderer3D::Init()
	{
		GE_PROFILE_FUNCTION();

		gsData = MakeOwn<SceneData>();

		// Bindings 1-3: Camera, Object, Lights.
		// Binding 0 is reserved for future use.
		gsData->CameraUBO = UniformBuffer::Create(sizeof(CameraUBOData), 1);
		gsData->ObjectUBO = UniformBuffer::Create(sizeof(ObjectUBOData), 2);
		gsData->LightUBO  = UniformBuffer::Create(sizeof(LightUBOData),  3);

		gsMeshShader = Shader::Create("assets/shaders/Mesh.glsl");

		// 1×1 white texture used when a mesh has no diffuse texture.
		TextureSpecification defaultSpec;
		defaultSpec.Width = 1;
		defaultSpec.Height = 1;
		defaultSpec.Format = ImageFormat::RGBA8;
		gsDefaultTexture = Texture2D::Create(defaultSpec);
		uint32_t whitePixel = 0xFFFFFFFF;
		gsDefaultTexture->SetData(&whitePixel, sizeof(uint32_t));

		// Upload a default (dark-ambient, no lights) environment so the buffer
		// is never uninitialized even if the caller skips SetLightEnvironment.
		SetLightEnvironment(LightEnvironment{});
	}

	void Renderer3D::Shutdown()
	{
		gsMeshShader.reset();
		gsDefaultTexture.reset();
		gsData.reset();
	}

	void Renderer3D::BeginScene(const EditorCamera& camera)
	{
		CameraUBOData cam;
		cam.ViewProjection = camera.GetViewProjection();
		cam.Position       = glm::vec4(camera.GetPosition(), 0.0f);
		gsData->CameraUBO->SetData(&cam, sizeof(CameraUBOData));
	}

	void Renderer3D::BeginScene(const glm::mat4& viewProjection, const glm::vec3& cameraPosition)
	{
		CameraUBOData cam;
		cam.ViewProjection = viewProjection;
		cam.Position       = glm::vec4(cameraPosition, 0.0f);
		gsData->CameraUBO->SetData(&cam, sizeof(CameraUBOData));
	}

	void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& cameraTransform)
	{
		CameraUBOData cam;
		cam.ViewProjection = camera.GetProjection() * glm::inverse(cameraTransform);
		cam.Position       = glm::vec4(glm::vec3(cameraTransform[3]), 0.0f);
		gsData->CameraUBO->SetData(&cam, sizeof(CameraUBOData));
	}

	void Renderer3D::EndScene()
	{
	}

	void Renderer3D::SetLightEnvironment(const LightEnvironment& env)
	{
		LightUBOData data{};

		data.AmbientColor = glm::vec4(env.AmbientColor, 0.0f);
		data.LightInfo.x  = env.HasDirectionalLight ? 1 : 0;

		if (env.HasDirectionalLight)
		{
			const auto& dl = env.DirectionalLight;
			data.DirLight.Direction = glm::vec4(dl.Direction, dl.Intensity);
			data.DirLight.Color     = glm::vec4(dl.Color, 0.0f);
		}

		int numPoint = (int)std::min((size_t)MaxPointLights, env.PointLights.size());
		data.LightInfo.y = numPoint;

		for (int i = 0; i < numPoint; i++)
		{
			const auto& pl = env.PointLights[i];
			data.PointLights[i].Position    = glm::vec4(pl.Position, pl.Intensity);
			data.PointLights[i].Color       = glm::vec4(pl.Color, pl.Constant);
			data.PointLights[i].Attenuation = glm::vec4(pl.Linear, pl.Quadratic, 0.0f, 0.0f);
		}

		gsData->LightUBO->SetData(&data, sizeof(LightUBOData));
	}

	void Renderer3D::Submit(const Handle<Mesh>& mesh, const glm::mat4& transform,
	                        const glm::vec4& color, int entityID)
	{
		if (!mesh)
			return;

		// Upload per-object data.
		ObjectUBOData obj{};
		obj.Transform = transform;
		obj.Color     = color;
		obj.EntityID  = entityID;
		gsData->ObjectUBO->SetData(&obj, sizeof(ObjectUBOData));

		// Bind diffuse texture (or white fallback) to slot 0.
		const auto& tex = mesh->GetTexture();
		(tex ? tex : gsDefaultTexture)->Bind(0);

		gsMeshShader->Bind();
		RenderCommand::DrawIndexed(mesh->GetVertexArray(), mesh->GetIndexCount());
	}

	Handle<Texture2D> Renderer3D::LoadTexture(const std::string& path)
	{
		return Application::GetInstance().GetAssetManager().LoadTexture(path);
	}

}
