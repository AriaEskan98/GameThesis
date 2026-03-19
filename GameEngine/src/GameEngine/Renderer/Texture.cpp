#include "gepch.h"
#include "GameEngine/Renderer/Texture.h"

#include "GameEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace GameEngine {

	Handle<Texture2D> Texture2D::Create(const TextureSpecification& specification)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return MakeHandle<OpenGLTexture2D>(specification);
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Handle<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return MakeHandle<OpenGLTexture2D>(path);
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Handle<Texture2D> TextureLibrary::Load(const std::string& path)
	{
		if (Exists(path))
			return myTextures[path];

		auto texture = Texture2D::Create(path);
		myTextures[path] = texture;
		return texture;
	}

	Handle<Texture2D> TextureLibrary::Get(const std::string& path)
	{
		GE_CORE_ASSERT(Exists(path), "Texture not found!");
		return myTextures[path];
	}

	bool TextureLibrary::Exists(const std::string& path) const
	{
		return myTextures.find(path) != myTextures.end();
	}

}
