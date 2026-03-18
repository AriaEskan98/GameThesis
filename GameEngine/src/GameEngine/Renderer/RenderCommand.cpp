#include "gepch.h"
#include "GameEngine/Renderer/RenderCommand.h"

namespace GameEngine {

	Own<RendererAPI> RenderCommand::gsRendererAPI = RendererAPI::Create();

}