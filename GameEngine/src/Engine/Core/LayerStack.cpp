#include "gepch.h"
#include "Engine/Core/LayerStack.h"

namespace GameEngine {

	LayerStack::~LayerStack()
	{
		for (Layer* layer : myLayers)
		{
			layer->OnDetach();
			delete layer;
		}
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		myLayers.emplace(myLayers.begin() + myLayerInsertIndex, layer);
		myLayerInsertIndex++;
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		myLayers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto it = std::find(myLayers.begin(), myLayers.begin() + myLayerInsertIndex, layer);
		if (it != myLayers.begin() + myLayerInsertIndex)
		{
			layer->OnDetach();
			myLayers.erase(it);
			myLayerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		auto it = std::find(myLayers.begin() + myLayerInsertIndex, myLayers.end(), overlay);
		if (it != myLayers.end())
		{
			overlay->OnDetach();
			myLayers.erase(it);
		}
	}

}
