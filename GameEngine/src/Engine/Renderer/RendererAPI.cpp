#include "gepch.h"
#include "Engine/Renderer/RendererAPI.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace GameEngine {

	RendererAPI::API RendererAPI::gsAPI = RendererAPI::API::OpenGL;

	Own<RendererAPI> RendererAPI::Create()
	{
		switch (gsAPI)
		{
			case RendererAPI::API::None:    GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return MakeOwn<OpenGLRendererAPI>();
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}