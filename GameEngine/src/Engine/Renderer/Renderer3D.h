#pragma once

#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/EditorCamera.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/UniformBuffer.h"

#include <glm/glm.hpp>

#include <vector>

namespace GameEngine {

	// -----------------------------------------------------------------------
	// Light descriptor types — filled by the Scene and passed to Renderer3D.
	// -----------------------------------------------------------------------

	/// Parameters of a single directional (sun) light.
	struct DirectionalLightData
	{
		glm::vec3 Direction = { 0.0f, -1.0f, 0.0f }; ///< World-space direction toward scene.
		glm::vec3 Color     = { 1.0f,  1.0f, 1.0f };
		float     Intensity = 1.0f;
	};

	/// Parameters of a single point light with quadratic attenuation.
	struct PointLightData
	{
		glm::vec3 Position  = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Color     = { 1.0f, 1.0f, 1.0f };
		float     Intensity = 1.0f;
		float     Constant  = 1.0f;
		float     Linear    = 0.09f;
		float     Quadratic = 0.032f;
	};

	/// All lighting information for one rendered frame.
	struct LightEnvironment
	{
		glm::vec3 AmbientColor = { 0.1f, 0.1f, 0.1f };

		bool                HasDirectionalLight = false;
		DirectionalLightData DirectionalLight;

		std::vector<PointLightData> PointLights; ///< Capped at MaxPointLights.
	};

	// -----------------------------------------------------------------------

	/// Renderer for 3D meshes using Blinn-Phong shading.
	/// All GPU data is passed via Uniform Buffer Objects at bindings 1-3,
	/// leaving binding 0 free for Renderer2D's camera buffer.
	class Renderer3D
	{
	public:
		static constexpr int MaxPointLights = 4;

		static void Init();
		static void Shutdown();

		/// Begin a 3D scene using the editor (viewport) camera.
		static void BeginScene(const EditorCamera& camera);

		/// Begin a 3D scene using a runtime camera and its entity world transform.
		static void BeginScene(const Camera& camera, const glm::mat4& cameraTransform);

		/// Begin a 3D scene with a pre-combined view-projection matrix and camera world position.
		/// Use this with FPSCameraController: BeginScene(ctrl.GetViewProjection(), ctrl.GetPosition()).
		static void BeginScene(const glm::mat4& viewProjection, const glm::vec3& cameraPosition);

		static void EndScene();

		/// Upload lighting data for the current scene. Must be called after BeginScene.
		static void SetLightEnvironment(const LightEnvironment& env);

		/// Submit a mesh for rendering with a world transform, flat base colour, and entity ID.
		static void Submit(const Handle<Mesh>& mesh, const glm::mat4& transform,
		                   const glm::vec4& color = glm::vec4(1.0f), int entityID = -1);

	private:
		// ---- GPU buffer layouts (must match GLSL std140 definitions in Mesh.glsl) ----

		struct CameraUBOData
		{
			glm::mat4 ViewProjection; // 64 bytes
			glm::vec4 Position;       // 16 bytes  (xyz = pos, w unused)
		};

		struct ObjectUBOData
		{
			glm::mat4 Transform;  // 64 bytes
			glm::vec4 Color;      // 16 bytes
			int EntityID;         //  4 bytes
			int _pad0, _pad1, _pad2; // 12 bytes padding to 96 total
		};

		/// Directional light entry inside the light UBO.
		struct DirLightGPU
		{
			glm::vec4 Direction; // xyz = direction, w = intensity
			glm::vec4 Color;     // xyz = colour,    w unused
		};

		/// Point light entry inside the light UBO.
		struct PointLightGPU
		{
			glm::vec4 Position;    // xyz = position,  w = intensity
			glm::vec4 Color;       // xyz = colour,    w = constant attenuation
			glm::vec4 Attenuation; // x = linear, y = quadratic, zw unused
		};

		struct LightUBOData
		{
			glm::vec4   AmbientColor;                      // 16 bytes
			glm::ivec4  LightInfo;                         // 16 bytes  (x=hasDir, y=numPoint)
			DirLightGPU DirLight;                          // 32 bytes
			PointLightGPU PointLights[MaxPointLights];     // 4 * 48 = 192 bytes
			// Total: 256 bytes
		};

		// ---- Internal state ----

		struct SceneData
		{
			Handle<UniformBuffer> CameraUBO;  // binding 1
			Handle<UniformBuffer> ObjectUBO;  // binding 2
			Handle<UniformBuffer> LightUBO;   // binding 3
		};

		static Own<SceneData>  gsData;
		static Handle<Shader>  gsMeshShader;
	};

}
