// Adapted from Hazel Engine by TheCherno
// Source: https://github.com/TheCherno/Hazel
// Changes: Renamed member variables to use "my" prefix convention;
//          added GetActiveWidgetID(); SetDarkThemeColors() moved from editor into this layer;
//          removed Layer inheritance — now a standalone ImGui management class
#pragma once

#include "GameEngine/Events/ApplicationEvent.h"
#include "GameEngine/Events/KeyEvent.h"
#include "GameEngine/Events/MouseEvent.h"

namespace GameEngine {

	class ImGuiLayer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		void OnAttach();
		void OnDetach();
		void OnEvent(Event& e);

		void Begin();
		void End();

		void BlockEvents(bool block) { myBlockEvents = block; }

		void SetDarkThemeColors();

		uint32_t GetActiveWidgetID() const;
	private:
		bool myBlockEvents = true;
	};

}
