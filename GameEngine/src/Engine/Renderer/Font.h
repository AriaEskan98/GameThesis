#pragma once

#include <filesystem>

#include "Engine/Core/Base.h"
#include "Engine/Renderer/Texture.h"

namespace GameEngine {

	struct MSDFData;

	class Font
	{
	public:
		Font(const std::filesystem::path& font);
		~Font();

		const MSDFData* GetMSDFData() const { return myData; }
		Handle<Texture2D> GetAtlasTexture() const { return myAtlasTexture; }

		static Handle<Font> GetDefault();
	private:
		MSDFData* myData;
		Handle<Texture2D> myAtlasTexture;
	};

}
