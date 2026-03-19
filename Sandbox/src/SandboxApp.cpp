// Original implementation
#include <GameEngine.h>
#include <GameEngine/Core/EntryPoint.h>

class Sandbox : public GameEngine::Application
{
public:
	Sandbox(const GameEngine::ApplicationSpecification& specification)
		: GameEngine::Application(specification)
	{
	}

protected:
	void OnUpdate(GameEngine::Timestep ts) override
	{
	}

	void OnImGuiRender() override
	{
	}

	void OnUserEvent(GameEngine::Event& e) override
	{
	}
};

GameEngine::Application* GameEngine::CreateApplication(GameEngine::ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "Sandbox";
	spec.WorkingDirectory = "../Sandbox";
	spec.CommandLineArgs = args;

	return new Sandbox(spec);
}
