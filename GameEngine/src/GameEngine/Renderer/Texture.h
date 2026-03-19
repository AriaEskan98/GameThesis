#pragma once

#include "GameEngine/Core/Base.h"

#include <string>
#include <unordered_map>

namespace GameEngine {

	enum class ImageFormat
	{
		None = 0,
		R8,
		RGB8,
		RGBA8,
		RGBA32F
	};

	struct TextureSpecification
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		ImageFormat Format = ImageFormat::RGBA8;
		bool GenerateMips = true;
	};

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual const TextureSpecification& GetSpecification() const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual const std::string& GetPath() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual bool IsLoaded() const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Handle<Texture2D> Create(const TextureSpecification& specification);
		static Handle<Texture2D> Create(const std::string& path);
	};

	class TextureLibrary
	{
	public:
		Handle<Texture2D> Load(const std::string& path);
		Handle<Texture2D> Get(const std::string& path);
		bool Exists(const std::string& path) const;
	private:
		std::unordered_map<std::string, Handle<Texture2D>> myTextures;
	};

}
