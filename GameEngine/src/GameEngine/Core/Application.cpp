// Adapted from Hazel Engine by TheCherno
// Source: https://github.com/TheCherno/Hazel
// Changes: Renamed member variables to use "my" prefix convention;
//          replaced LayerStack with Unity-style manager classes
#include "gepch.h"
#include "GameEngine/Core/Application.h"

#include "GameEngine/Core/Log.h"

#include "GameEngine/Renderer/Renderer.h"

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

		if (!mySpecification.WorkingDirectory.empty())
			std::filesystem::current_path(mySpecification.WorkingDirectory);

		myWindow = Window::Create(WindowProps(mySpecification.Name));
		myWindow->SetEventCallback([this](Event& e) { OnEvent(e); });

		Renderer::Init();

		// Initialise all managers (AssetManager first — needs GL context, must
		// outlive the renderer so GPU resources are freed before GL shutdown)
		myAssetManager   = MakeOwn<AssetManager>();
		myPhysicsManager = MakeOwn<PhysicsManager>();
		mySceneManager   = MakeOwn<SceneManager>();
		myRenderManager  = MakeOwn<RenderManager>();
		myUIManager      = MakeOwn<UIManager>();

		myAssetManager->Init();
		myPhysicsManager->Init();
		mySceneManager->Init();
		myRenderManager->Init();
		myUIManager->Init();
	}

	Application::~Application()
	{
		GE_PROFILE_FUNCTION();

		myUIManager->Shutdown();
		myRenderManager->Shutdown();
		mySceneManager->Shutdown();
		myPhysicsManager->Shutdown();
		myAssetManager->Shutdown();   // after all systems that use assets

		Renderer::Shutdown();
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

		// UI gets first pick — if it wants the event, game doesn't see it
		if (myUIManager->OnEvent(e))
			return;

		OnUserEvent(e);
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
				// Unity-style update order:
				myPhysicsManager->Update(timestep);   // 1. Physics (FixedUpdate equivalent)
				mySceneManager->Update(timestep);     // 2. Scene systems (animation, etc.)
				OnUpdate(timestep);                   // 3. Game logic
				myRenderManager->Render();            // 4. 3D / 2D render
				myUIManager->Render([this]() { OnImGuiRender(); }); // 5. UI always last
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
