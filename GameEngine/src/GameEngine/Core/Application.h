#pragma once

#include "GameEngine/Core/Base.h"

#include "GameEngine/Core/Window.h"
#include "GameEngine/Events/Event.h"
#include "GameEngine/Events/ApplicationEvent.h"

#include "GameEngine/Core/Timestep.h"

#include "GameEngine/ImGui/ImGuiLayer.h"

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

		ImGuiLayer* GetImGuiLayer() { return myImGuiLayer; }

		static Application& GetInstance() { return *gsInstance; }

		const ApplicationSpecification& GetSpecification() const { return mySpecification; }

		void SubmitToMainThread(const std::function<void()>& function);

	protected:
		// Override these in your Application subclass
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
		ImGuiLayer* myImGuiLayer;
		bool myRunning = true;
		bool myMinimized = false;
		float myLastFrameTime = 0.0f;

		std::vector<std::function<void()>> myMainThreadQueue;
		std::mutex myMainThreadQueueMutex;
	private:
		static Application* gsInstance;
		friend int ::main(int argc, char** argv);
	};

	// To be defined in CLIENT
	Application* CreateApplication(ApplicationCommandLineArgs args);

}
