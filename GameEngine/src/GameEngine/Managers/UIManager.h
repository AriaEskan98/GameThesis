// Original implementation
#pragma once

#include "GameEngine/Core/Base.h"
#include "GameEngine/ImGui/ImGuiLayer.h"
#include "GameEngine/Events/Event.h"

#include <functional>

namespace GameEngine {

	// Owns the ImGui context and manages its full lifecycle.
	// Sits last in the update order so UI always renders on top.
	class UIManager
	{
	public:
		void Init();
		void Shutdown();

		// Returns true if ImGui consumed the event (game should not see it).
		bool OnEvent(Event& e);

		// Wraps ImGui's Begin/End frame around the caller's render function.
		void Render(const std::function<void()>& renderFn);

		ImGuiLayer* GetImGuiLayer() { return myImGuiLayer.get(); }

	private:
		Own<ImGuiLayer> myImGuiLayer;
	};

}
