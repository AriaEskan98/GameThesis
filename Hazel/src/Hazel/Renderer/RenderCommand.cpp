#include "gepch.h"
#include "Hazel/Renderer/RenderCommand.h"

namespace GameEngine {

	Own<RendererAPI> RenderCommand::gsRendererAPI = RendererAPI::Create();

}