// Adapted from Hazel Engine by TheCherno
// Source: https://github.com/TheCherno/Hazel
// Changes: Renamed member variables to use "my" prefix convention; refactored internal
//          storage to use separate myLayerList/myOverlayList with RebuildCombined()
//          instead of Hazel's insert-position iterator approach
#include "gepch.h"
#include "GameEngine/Core/LayerStack.h"

namespace GameEngine {

	LayerStack::~LayerStack()
	{
		for (Layer* layer : myAll)
		{
			layer->OnDetach();
			delete layer;
		}
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		myLayerList.push_back(layer);
		RebuildCombined();
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		myOverlayList.push_back(overlay);
		RebuildCombined();
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto it = std::find(myLayerList.begin(), myLayerList.end(), layer);
		if (it != myLayerList.end())
		{
			layer->OnDetach();
			myLayerList.erase(it);
			RebuildCombined();
		}
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		auto it = std::find(myOverlayList.begin(), myOverlayList.end(), overlay);
		if (it != myOverlayList.end())
		{
			overlay->OnDetach();
			myOverlayList.erase(it);
			RebuildCombined();
		}
	}

	void LayerStack::RebuildCombined()
	{
		myAll.clear();
		myAll.insert(myAll.end(), myLayerList.begin(), myLayerList.end());
		myAll.insert(myAll.end(), myOverlayList.begin(), myOverlayList.end());
	}

}
