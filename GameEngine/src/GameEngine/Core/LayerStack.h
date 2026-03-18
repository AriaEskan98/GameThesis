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

		std::vector<Layer*>::iterator begin() { return myLayers.begin(); }
		std::vector<Layer*>::iterator end() { return myLayers.end(); }
		std::vector<Layer*>::reverse_iterator rbegin() { return myLayers.rbegin(); }
		std::vector<Layer*>::reverse_iterator rend() { return myLayers.rend(); }

		std::vector<Layer*>::const_iterator begin() const { return myLayers.begin(); }
		std::vector<Layer*>::const_iterator end()	const { return myLayers.end(); }
		std::vector<Layer*>::const_reverse_iterator rbegin() const { return myLayers.rbegin(); }
		std::vector<Layer*>::const_reverse_iterator rend() const { return myLayers.rend(); }
	private:
		std::vector<Layer*> myLayers;
		unsigned int myLayerInsertIndex = 0;
	};

}