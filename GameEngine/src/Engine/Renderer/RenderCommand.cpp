#include "gepch.h"
#include "Engine/Renderer/RenderCommand.h"

namespace GameEngine {

	Own<RendererAPI> RenderCommand::gsRendererAPI = RendererAPI::Create();

}