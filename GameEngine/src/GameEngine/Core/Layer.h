// Adapted from Hazel Engine by TheCherno
// Source: https://github.com/TheCherno/Hazel
// Changes: Renamed member variables to use "my" prefix convention
#pragma once

#include "GameEngine/Core/Base.h"
#include "GameEngine/Core/Timestep.h"
#include "GameEngine/Events/Event.h"

namespace GameEngine {

	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		const std::string& GetName() const { return myDebugName; }
	protected:
		std::string myDebugName;
	};

}