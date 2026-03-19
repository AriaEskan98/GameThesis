// Original implementation
#pragma once

#include "GameEngine/Core/Timestep.h"

namespace GameEngine {

	// Physics simulation is currently managed inside Scene (Box2D / Physics3D).
	// This manager is the designated home for future extraction of physics into
	// a standalone system with fixed timestep, substeps, and global queries.
	class PhysicsManager
	{
	public:
		void Init() {}
		void Shutdown() {}
		void Update(Timestep ts) {}  // No-op: physics ticks inside Scene::OnUpdateRuntime
	};

}
