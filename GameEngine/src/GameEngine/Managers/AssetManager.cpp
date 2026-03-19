#include "gepch.h"
#include "GameEngine/Managers/AssetManager.h"

namespace GameEngine {

	void AssetManager::Init()
	{
		GE_CORE_INFO("AssetManager: initialized");
	}

	void AssetManager::Shutdown()
	{
		// Release all cached assets. Shared pointers will drop to zero if no
		// one else holds a reference, freeing GPU resources while the GL
		// context is still alive (Application shuts us down before Renderer).
		myMeshes.clear();
		myTextures = TextureLibrary{};
		myShaders  = ShaderLibrary{};
		GE_CORE_INFO("AssetManager: all assets released");
	}

	// ---- Textures -----------------------------------------------------------

	Handle<Texture2D> AssetManager::LoadTexture(const std::string& path)
	{
		return myTextures.Load(path);
	}

	Handle<Texture2D> AssetManager::GetTexture(const std::string& path) const
	{
		GE_CORE_ASSERT(TextureExists(path), "AssetManager: texture not loaded: {0}", path);
		// TextureLibrary::Get is non-const, so reach into the map directly.
		// Cast away const — map lookup on an existing key does not modify state.
		return const_cast<AssetManager*>(this)->myTextures.Get(path);
	}

	bool AssetManager::TextureExists(const std::string& path) const
	{
		return myTextures.Exists(path);
	}

	// ---- Shaders ------------------------------------------------------------

	Handle<Shader> AssetManager::LoadShader(const std::string& path)
	{
		return myShaders.Load(path);
	}

	Handle<Shader> AssetManager::LoadShader(const std::string& name, const std::string& path)
	{
		return myShaders.Load(name, path);
	}

	Handle<Shader> AssetManager::GetShader(const std::string& name) const
	{
		GE_CORE_ASSERT(ShaderExists(name), "AssetManager: shader not loaded: {0}", name);
		return const_cast<AssetManager*>(this)->myShaders.Get(name);
	}

	bool AssetManager::ShaderExists(const std::string& name) const
	{
		return myShaders.Exists(name);
	}

	// ---- Meshes -------------------------------------------------------------

	Handle<Mesh> AssetManager::LoadMesh(const std::string& path)
	{
		if (MeshExists(path))
			return myMeshes[path];

		auto mesh = Mesh::Create(path);   // also triggers texture load internally
		myMeshes[path] = mesh;
		return mesh;
	}

	Handle<Mesh> AssetManager::GetMesh(const std::string& path) const
	{
		GE_CORE_ASSERT(MeshExists(path), "AssetManager: mesh not loaded: {0}", path);
		return myMeshes.at(path);
	}

	bool AssetManager::MeshExists(const std::string& path) const
	{
		return myMeshes.find(path) != myMeshes.end();
	}

}
