// Original implementation
#include <GameEngine.h>
#include <GameEngine/Core/EntryPoint.h>

#include "Sandbox2D.h"

class Sandbox : public GameEngine::Application
{
public:
	Sandbox(const GameEngine::ApplicationSpecification& specification)
		: GameEngine::Application(specification)
	{
		mySandbox2D.OnAttach();
	}

	~Sandbox()
	{
		mySandbox2D.OnDetach();
	}

protected:
	void OnUpdate(GameEngine::Timestep ts) override
	{
		mySandbox2D.OnUpdate(ts);
	}

	void OnImGuiRender() override
	{
		mySandbox2D.OnImGuiRender();
	}

	void OnUserEvent(GameEngine::Event& e) override
	{
		mySandbox2D.OnEvent(e);
	}

private:
	Sandbox2D mySandbox2D;
};

GameEngine::Application* GameEngine::CreateApplication(GameEngine::ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "Sandbox";
	spec.WorkingDirectory = "../Sandbox";
	spec.CommandLineArgs = args;

	return new Sandbox(spec);
}
