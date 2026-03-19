#pragma once

#include "GameEngine/Core/Base.h"

#include "GameEngine/Core/Window.h"
#include "GameEngine/Events/Event.h"
#include "GameEngine/Events/ApplicationEvent.h"

#include "GameEngine/Core/Timestep.h"

#include "GameEngine/Managers/UIManager.h"
#include "GameEngine/Managers/SceneManager.h"
#include "GameEngine/Managers/PhysicsManager.h"
#include "GameEngine/Managers/RenderManager.h"
#include "GameEngine/Managers/AudioManager.h"

int main(int argc, char** argv);

namespace GameEngine {

	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			GE_CORE_ASSERT(index < Count);
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		std::string Name = "GameEngine Application";
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void OnEvent(Event& e);

		Window& GetWindow() { return *myWindow; }

		void Close();

		SceneManager&  GetSceneManager()  { return *mySceneManager; }
		UIManager&     GetUIManager()     { return *myUIManager; }
		PhysicsManager& GetPhysicsManager() { return *myPhysicsManager; }
		RenderManager& GetRenderManager() { return *myRenderManager; }
		AudioManager&  GetAudioManager()  { return *myAudioManager; }

		// Convenience: direct access to ImGui layer via UIManager
		ImGuiLayer* GetImGuiLayer() { return myUIManager->GetImGuiLayer(); }

		static Application& GetInstance() { return *gsInstance; }

		const ApplicationSpecification& GetSpecification() const { return mySpecification; }

		void SubmitToMainThread(const std::function<void()>& function);

	protected:
		// Game-code hooks — override in your Application subclass.
		// These run between the engine managers in the Unity-style update order.
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnUserEvent(Event& e) {}

	private:
		void Run();
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		void ExecuteMainThreadQueue();
	private:
		ApplicationSpecification mySpecification;
		Own<Window> myWindow;
		bool myRunning = true;
		bool myMinimized = false;
		float myLastFrameTime = 0.0f;

		// Managers — called in fixed Unity-style order each frame
		Own<PhysicsManager> myPhysicsManager;
		Own<SceneManager>   mySceneManager;
		Own<RenderManager>  myRenderManager;
		Own<UIManager>      myUIManager;
		Own<AudioManager>   myAudioManager;

		std::vector<std::function<void()>> myMainThreadQueue;
		std::mutex myMainThreadQueueMutex;
	private:
		static Application* gsInstance;
		friend int ::main(int argc, char** argv);
	};

	// To be defined in CLIENT
	Application* CreateApplication(ApplicationCommandLineArgs args);

}
