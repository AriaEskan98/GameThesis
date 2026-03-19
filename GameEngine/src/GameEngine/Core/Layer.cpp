// Adapted from Hazel Engine by TheCherno
// Source: https://github.com/TheCherno/Hazel
// Changes: Renamed member variables to use "my" prefix convention
#include "gepch.h"
#include "GameEngine/Core/Layer.h"

namespace GameEngine {

	Layer::Layer(const std::string& debugName)
		: myDebugName(debugName)
	{
	}
	
}