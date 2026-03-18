#include <Engine.h>
#include <Engine/Core/EntryPoint.h>

#include "Sandbox2D.h"
#include "ExampleLayer.h"

class Sandbox : public GameEngine::Application
{
public:
	Sandbox(const GameEngine::ApplicationSpecification& specification)
		: GameEngine::Application(specification)
	{
		// PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}

	~Sandbox()
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
