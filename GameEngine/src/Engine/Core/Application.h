#pragma once

#include "Engine/Core/Base.h"

#include "Engine/Core/Window.h"
#include "Engine/Core/LayerStack.h"
#include "Engine/Events/Event.h"
#include "Engine/Events/ApplicationEvent.h"

#include "Engine/Core/Timestep.h"

#include "Engine/ImGui/ImGuiLayer.h"

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

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		Window& GetWindow() { return *myWindow; }

		void Close();

		ImGuiLayer* GetImGuiLayer() { return myImGuiLayer; }

		static Application& GetInstance() { return *gsInstance; }

		const ApplicationSpecification& GetSpecification() const { return mySpecification; }

		void SubmitToMainThread(const std::function<void()>& function);
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
		LayerStack myLayerStack;
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
