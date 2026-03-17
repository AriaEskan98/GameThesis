#pragma once

#include "Hazel.h"

class Sandbox2D : public GameEngine::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(GameEngine::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(GameEngine::Event& e) override;
private:
	GameEngine::OrthographicCameraController myCameraController;

	// Temp
	GameEngine::Handle<GameEngine::VertexArray> mySquareVA;
	GameEngine::Handle<GameEngine::Shader> myFlatColorShader;

	GameEngine::Handle<GameEngine::Texture2D> myCheckerboardTexture;

	glm::vec4 mySquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};
