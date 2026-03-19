#include "gepch.h"
#include "GameEngine/Core/Application.h"

#include "GameEngine/Core/Log.h"

#include "GameEngine/Renderer/Renderer.h"
#include "GameEngine/Scripting/ScriptEngine.h"

#include "GameEngine/Core/Input.h"
#include "GameEngine/Utils/PlatformUtils.h"

namespace GameEngine {

	Application* Application::gsInstance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: mySpecification(specification)
	{
		GE_PROFILE_FUNCTION();

		GE_CORE_ASSERT(!gsInstance, "Application already exists!");
		gsInstance = this;

		// Set working directory here
		if (!mySpecification.WorkingDirectory.empty())
			std::filesystem::current_path(mySpecification.WorkingDirectory);

		myWindow = Window::Create(WindowProps(mySpecification.Name));
		myWindow->SetEventCallback([this](Event& e) { OnEvent(e); });

		Renderer::Init();

		myImGuiLayer = new ImGuiLayer();
		PushOverlay(myImGuiLayer);
	}

	Application::~Application()
	{
		GE_PROFILE_FUNCTION();

		ScriptEngine::Shutdown();
		Renderer::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		GE_PROFILE_FUNCTION();

		myLayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		GE_PROFILE_FUNCTION();

		myLayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::Close()
	{
		myRunning = false;
	}

	void Application::SubmitToMainThread(const std::function<void()>& function)
	{
		std::scoped_lock<std::mutex> lock(myMainThreadQueueMutex);
		myMainThreadQueue.push_back(function);
	}

	void Application::OnEvent(Event& e)
	{
		GE_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return OnWindowClose(e); });
		dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return OnWindowResize(e); });

		for (auto it = myLayerStack.rbegin(); it != myLayerStack.rend() && !e.Handled; ++it)
			(*it)->OnEvent(e);
	}

	void Application::Run()
	{
		GE_PROFILE_FUNCTION();

		while (myRunning)
		{
			GE_PROFILE_SCOPE("RunLoop");

			const float currentTime = Time::GetTime();
			Timestep timestep = currentTime - myLastFrameTime;
			myLastFrameTime = currentTime;

			ExecuteMainThreadQueue();

			if (!myMinimized)
			{
				for (Layer* layer : myLayerStack)
					layer->OnUpdate(timestep);

				myImGuiLayer->Begin();
				for (Layer* layer : myLayerStack)
					layer->OnImGuiRender();
				myImGuiLayer->End();
			}

			myWindow->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		myRunning = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		GE_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			myMinimized = true;
			return false;
		}

		myMinimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

	void Application::ExecuteMainThreadQueue()
	{
		std::vector<std::function<void()>> queue;
		{
			std::scoped_lock<std::mutex> lock(myMainThreadQueueMutex);
			queue.swap(myMainThreadQueue);
		}

		for (auto& func : queue)
			func();
	}

}
