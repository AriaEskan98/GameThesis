// Original implementation
#pragma once

#include "GameEngine/Core/Base.h"
#include "GameEngine/Core/Timestep.h"
#include "GameEngine/Scene/Scene.h"

namespace GameEngine {

	// Manages the active scene and its runtime lifecycle.
	// Scene-level systems (scripting, animation) would tick here in Update().
	class SceneManager
	{
	public:
		void Init() {}
		void Shutdown() {}

		void SetActiveScene(Handle<Scene> scene);
		Handle<Scene> GetActiveScene() const { return myActiveScene; }

		void OnRuntimeStart();
		void OnRuntimeStop();

		// Ticks scene-level engine systems (not game logic — that lives in OnUpdate).
		void Update(Timestep ts);

	private:
		Handle<Scene> myActiveScene;
		bool myIsRunning = false;
	};

}
