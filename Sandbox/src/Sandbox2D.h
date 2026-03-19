// Original implementation
#pragma once

#include "GameEngine.h"

class Sandbox2D
{
public:
	Sandbox2D();
	~Sandbox2D() = default;

	void OnAttach();
	void OnDetach();

	void OnUpdate(GameEngine::Timestep ts);
	void OnImGuiRender();
	void OnEvent(GameEngine::Event& e);
private:
	GameEngine::OrthographicCameraController myCameraController;

	// Temp
	GameEngine::Handle<GameEngine::VertexArray> mySquareVA;
	GameEngine::Handle<GameEngine::Shader> myFlatColorShader;

	GameEngine::Handle<GameEngine::Texture2D> myCheckerboardTexture;

	// Per-quad colors: 0 = rotated red, 1 = flat red, 2 = blue
	glm::vec4 mySquareColors[3] = {
		{ 0.8f, 0.2f, 0.3f, 1.0f },
		{ 0.8f, 0.2f, 0.3f, 1.0f },
		{ 0.2f, 0.3f, 0.8f, 1.0f }
	};
	int mySelectedSquare = -1;
	glm::vec4 myBackgroundColor = { 0.1f, 0.1f, 0.1f, 1.0f };

	bool OnMouseButtonPressed(GameEngine::MouseButtonPressedEvent& e);
};
