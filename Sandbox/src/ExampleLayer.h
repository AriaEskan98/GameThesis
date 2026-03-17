#pragma once

#include "Hazel.h"

class ExampleLayer : public GameEngine::Layer
{
public:
	ExampleLayer();
	virtual ~ExampleLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(GameEngine::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(GameEngine::Event& e) override;
private:
	GameEngine::ShaderLibrary myShaderLibrary;
	GameEngine::Handle<GameEngine::Shader> myShader;
	GameEngine::Handle<GameEngine::VertexArray> myVertexArray;

	GameEngine::Handle<GameEngine::Shader> myFlatColorShader;
	GameEngine::Handle<GameEngine::VertexArray> mySquareVA;

	GameEngine::Handle<GameEngine::Texture2D> myTexture, myChernoLogoTexture;

	GameEngine::OrthographicCameraController myCameraController;
	glm::vec3 mySquareColor = { 0.2f, 0.3f, 0.8f };
};
