// Original implementation
#include "gepch.h"
#include "SceneManager.h"

namespace GameEngine {

	void SceneManager::SetActiveScene(Handle<Scene> scene)
	{
		myActiveScene = scene;
	}

	void SceneManager::OnRuntimeStart()
	{
		if (myActiveScene)
		{
			myActiveScene->OnRuntimeStart();
			myIsRunning = true;
		}
	}

	void SceneManager::OnRuntimeStop()
	{
		if (myActiveScene && myIsRunning)
		{
			myActiveScene->OnRuntimeStop();
			myIsRunning = false;
		}
	}

	void SceneManager::Update(Timestep ts)
	{
		// Scene-level engine systems (e.g. animation, scripting) would tick here.
		// Game logic runs separately via Application::OnUpdate.
	}

}
