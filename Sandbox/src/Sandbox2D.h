#pragma once

#include "Hazel.h"

class Sandbox2D : public Hazel::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(Hazel::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(Hazel::Event& e) override;
private:
	Hazel::OrthographicCameraController myCameraController;

	// Temp
	Hazel::Handle<Hazel::VertexArray> mySquareVA;
	Hazel::Handle<Hazel::Shader> myFlatColorShader;

	Hazel::Handle<Hazel::Texture2D> myCheckerboardTexture;

	glm::vec4 mySquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};
