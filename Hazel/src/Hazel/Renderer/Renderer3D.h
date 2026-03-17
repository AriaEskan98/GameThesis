#pragma once

#include "Hazel/Renderer/Mesh.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Renderer/Camera.h"

#include <glm/glm.hpp>

namespace GameEngine {

	/// Renderer for 3D meshes. Operates independently from Renderer2D and must be
	/// surrounded by BeginScene / EndScene calls per frame. Meshes are submitted
	/// individually; no vertex batching is performed (mesh data is static on the GPU).
	class Renderer3D
	{
	public:
		static void Init();
		static void Shutdown();

		/// Begin a 3D scene using the editor camera (viewport camera in editor mode).
		static void BeginScene(const EditorCamera& camera);

		/// Begin a 3D scene using a runtime scene camera and its world transform.
		static void BeginScene(const Camera& camera, const glm::mat4& cameraTransform);

		static void EndScene();

		/// Submit a mesh for rendering with the given world transform, flat colour, and entity ID.
		static void Submit(const Handle<Mesh>& mesh, const glm::mat4& transform,
		                   const glm::vec4& color = glm::vec4(1.0f), int entityID = -1);

	private:
		struct SceneData
		{
			glm::mat4 ViewProjection;
		};
		static Own<SceneData> gsSceneData;

		static Handle<Shader> gsMeshShader;
	};

}
