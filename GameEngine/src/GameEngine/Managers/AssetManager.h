#pragma once

#include "GameEngine/Core/Base.h"
#include "GameEngine/Renderer/Mesh.h"
#include "GameEngine/Renderer/Shader.h"
#include "GameEngine/Renderer/Texture.h"

#include <string>
#include <unordered_map>

namespace GameEngine {

	/// Centralised asset cache for the engine.
	/// All resource loading should go through here so that the same file on disk
	/// is never loaded or uploaded to the GPU more than once per session.
	///
	/// Usage (game code):
	///   auto& assets = Application::GetInstance().GetAssetManager();
	///   Handle<Mesh>      mesh    = assets.LoadMesh("assets/models/chair.obj");
	///   Handle<Texture2D> diffuse = assets.LoadTexture("assets/textures/wood.png");
	///   Handle<Shader>    shader  = assets.LoadShader("assets/shaders/Mesh.glsl");
	class AssetManager
	{
	public:
		void Init();
		void Shutdown();

		// ---- Textures -------------------------------------------------------

		/// Load a texture from disk, returning a cached copy on subsequent calls.
		Handle<Texture2D> LoadTexture(const std::string& path);

		/// Retrieve an already-loaded texture (asserts if not found).
		Handle<Texture2D> GetTexture(const std::string& path) const;

		bool TextureExists(const std::string& path) const;

		// ---- Shaders --------------------------------------------------------

		/// Load a shader from a single combined GLSL file (detected by Renderer).
		Handle<Shader> LoadShader(const std::string& path);

		/// Load a shader under an explicit name.
		Handle<Shader> LoadShader(const std::string& name, const std::string& path);

		/// Retrieve an already-loaded shader by name (asserts if not found).
		Handle<Shader> GetShader(const std::string& name) const;

		bool ShaderExists(const std::string& name) const;

		// ---- Meshes ---------------------------------------------------------

		/// Load a mesh from disk (Assimp), returning a cached copy on subsequent calls.
		/// Also automatically loads the diffuse texture referenced in the mesh material.
		Handle<Mesh> LoadMesh(const std::string& path);

		/// Retrieve an already-loaded mesh (asserts if not found).
		Handle<Mesh> GetMesh(const std::string& path) const;

		bool MeshExists(const std::string& path) const;

	private:
		TextureLibrary myTextures;
		ShaderLibrary  myShaders;
		std::unordered_map<std::string, Handle<Mesh>> myMeshes;
	};

}
