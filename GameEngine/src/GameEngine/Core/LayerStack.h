#pragma once

#include "GameEngine/Core/Base.h"
#include "GameEngine/Core/Layer.h"

#include <vector>

namespace GameEngine {

	class LayerStack
	{
	public:
		LayerStack() = default;
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		std::vector<Layer*>::iterator begin() { return myAll.begin(); }
		std::vector<Layer*>::iterator end() { return myAll.end(); }
		std::vector<Layer*>::reverse_iterator rbegin() { return myAll.rbegin(); }
		std::vector<Layer*>::reverse_iterator rend() { return myAll.rend(); }

		std::vector<Layer*>::const_iterator begin() const { return myAll.begin(); }
		std::vector<Layer*>::const_iterator end()   const { return myAll.end(); }
		std::vector<Layer*>::const_reverse_iterator rbegin() const { return myAll.rbegin(); }
		std::vector<Layer*>::const_reverse_iterator rend() const { return myAll.rend(); }

	private:
		void RebuildCombined();

	private:
		std::vector<Layer*> myLayerList;
		std::vector<Layer*> myOverlayList;
		std::vector<Layer*> myAll;
	};

}
