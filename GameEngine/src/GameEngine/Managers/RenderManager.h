// Original implementation
#pragma once

namespace GameEngine {

	// Rendering is currently managed inside Scene (Renderer3D).
	// This manager is the designated home for future extraction of rendering into
	// a standalone system with render graphs, culling, and batching control.
	class RenderManager
	{
	public:
		void Init() {}
		void Shutdown() {}
		void Render() {}  // No-op: rendering ticks inside Scene::OnUpdateRuntime / game code
	};

}
