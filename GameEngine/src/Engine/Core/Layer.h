#pragma once

#include "Engine/Core/Base.h"
#include "Engine/Core/Timestep.h"
#include "Engine/Events/Event.h"

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