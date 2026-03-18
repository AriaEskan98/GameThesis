#include "gepch.h"
#include "Hazel/Core/Application.h"

#include "Hazel/Core/Log.h"

#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Scripting/ScriptEngine.h"

#include "Hazel/Core/Input.h"
#include "Hazel/Utils/PlatformUtils.h"

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
		myWindow->SetEventCallback(GE_BIND_FN(Application::OnEvent));

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

		myMainThreadQueue.emplace_back(function);
	}

	void Application::OnEvent(Event& e)
	{
		GE_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(GE_BIND_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(GE_BIND_FN(Application::OnWindowResize));

		for (auto it = myLayerStack.rbegin(); it != myLayerStack.rend(); ++it)
		{
			if (e.Handled) 
				break;
			(*it)->OnEvent(e);
		}
	}

	void Application::Run()
	{
		GE_PROFILE_FUNCTION();

		while (myRunning)
		{
			GE_PROFILE_SCOPE("RunLoop");

			float time = Time::GetTime();
			Timestep timestep = time - myLastFrameTime;
			myLastFrameTime = time;

			ExecuteMainThreadQueue();

			if (!myMinimized)
			{
				{
					GE_PROFILE_SCOPE("LayerStack OnUpdate");

					for (Layer* layer : myLayerStack)
						layer->OnUpdate(timestep);
				}

				myImGuiLayer->Begin();
				{
					GE_PROFILE_SCOPE("LayerStack OnImGuiRender");

					for (Layer* layer : myLayerStack)
						layer->OnImGuiRender();
				}
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
		std::scoped_lock<std::mutex> lock(myMainThreadQueueMutex);

		for (auto& func : myMainThreadQueue)
			func();

		myMainThreadQueue.clear();
	}

}
