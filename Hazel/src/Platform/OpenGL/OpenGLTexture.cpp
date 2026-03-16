#include "gepch.h"
#include "Platform/OpenGL/OpenGLTexture.h"

#include <stb_image.h>

namespace GameEngine {

	namespace Utils {

		static GLenum HazelImageFormatToGLDataFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RGB8:  return GL_RGB;
				case ImageFormat::RGBA8: return GL_RGBA;
			}

			GE_CORE_ASSERT(false);
			return 0;
		}
		
		static GLenum HazelImageFormatToGLInternalFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB8:  return GL_RGB8;
			case ImageFormat::RGBA8: return GL_RGBA8;
			}

			GE_CORE_ASSERT(false);
			return 0;
		}

	}

	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification)
		: mySpecification(specification), myWidth(mySpecification.Width), myHeight(mySpecification.Height)
	{
		GE_PROFILE_FUNCTION();

		myInternalFormat = Utils::HazelImageFormatToGLInternalFormat(mySpecification.Format);
		myDataFormat = Utils::HazelImageFormatToGLDataFormat(mySpecification.Format);

		glCreateTextures(GL_TEXTURE_2D, 1, &myRendererID);
		glTextureStorage2D(myRendererID, 1, myInternalFormat, myWidth, myHeight);

		glTextureParameteri(myRendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(myRendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(myRendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(myRendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		: myPath(path)
	{
		GE_PROFILE_FUNCTION();

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;
		{
			GE_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std::string&)");
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}
			
		if (data)
		{
			myIsLoaded = true;

			myWidth = width;
			myHeight = height;

			GLenum internalFormat = 0, dataFormat = 0;
			if (channels == 4)
			{
				internalFormat = GL_RGBA8;
				dataFormat = GL_RGBA;
			}
			else if (channels == 3)
			{
				internalFormat = GL_RGB8;
				dataFormat = GL_RGB;
			}

			myInternalFormat = internalFormat;
			myDataFormat = dataFormat;

			GE_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");

			glCreateTextures(GL_TEXTURE_2D, 1, &myRendererID);
			glTextureStorage2D(myRendererID, 1, internalFormat, myWidth, myHeight);

			glTextureParameteri(myRendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(myRendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTextureParameteri(myRendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(myRendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTextureSubImage2D(myRendererID, 0, 0, 0, myWidth, myHeight, dataFormat, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		GE_PROFILE_FUNCTION();

		glDeleteTextures(1, &myRendererID);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		GE_PROFILE_FUNCTION();

		uint32_t bpp = myDataFormat == GL_RGBA ? 4 : 3;
		GE_CORE_ASSERT(size == myWidth * myHeight * bpp, "Data must be entire texture!");
		glTextureSubImage2D(myRendererID, 0, 0, 0, myWidth, myHeight, myDataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		GE_PROFILE_FUNCTION();

		glBindTextureUnit(slot, myRendererID);
	}
}
