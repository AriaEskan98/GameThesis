#pragma once
#include "Engine/Core/Base.h"
#include "Engine/Core/Application.h"

#ifdef GE_PLATFORM_WINDOWS

extern GameEngine::Application* GameEngine::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
	GameEngine::Log::Init();

	GE_PROFILE_BEGIN_SESSION("Startup", "EngineProfile-Startup.json");
	auto app = GameEngine::CreateApplication({ argc, argv });
	GE_PROFILE_END_SESSION();

	GE_PROFILE_BEGIN_SESSION("Runtime", "EngineProfile-Runtime.json");
	app->Run();
	GE_PROFILE_END_SESSION();

	GE_PROFILE_BEGIN_SESSION("Shutdown", "EngineProfile-Shutdown.json");
	delete app;
	GE_PROFILE_END_SESSION();
}

#endif
