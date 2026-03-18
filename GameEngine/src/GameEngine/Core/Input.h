#pragma once

#include "GameEngine/Core/KeyCodes.h"
#include "GameEngine/Core/MouseCodes.h"

#include <glm/glm.hpp>

namespace GameEngine {

	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);

		static bool IsMouseButtonPressed(MouseCode button);
		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}
