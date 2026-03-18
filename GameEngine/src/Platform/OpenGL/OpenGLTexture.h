#pragma once

#include "Engine/Renderer/Texture.h"

#include <glad/glad.h>

namespace GameEngine {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const TextureSpecification& specification);
		OpenGLTexture2D(const std::string& path);
		virtual ~OpenGLTexture2D();

		virtual const TextureSpecification& GetSpecification() const override { return mySpecification; }

		virtual uint32_t GetWidth() const override { return myWidth;  }
		virtual uint32_t GetHeight() const override { return myHeight; }
		virtual uint32_t GetRendererID() const override { return myRendererID; }

		virtual const std::string& GetPath() const override { return myPath; }
		
		virtual void SetData(void* data, uint32_t size) override;

		virtual void Bind(uint32_t slot = 0) const override;

		virtual bool IsLoaded() const override { return myIsLoaded; }

		virtual bool operator==(const Texture& other) const override
		{
			return myRendererID == other.GetRendererID();
		}
	private:
		TextureSpecification mySpecification;

		std::string myPath;
		bool myIsLoaded = false;
		uint32_t myWidth, myHeight;
		uint32_t myRendererID;
		GLenum myInternalFormat, myDataFormat;
	};

}
