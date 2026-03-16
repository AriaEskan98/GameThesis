#include "gepch.h"
#include "Hazel/Renderer/RenderCommand.h"

namespace GameEngine {

	Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();

}