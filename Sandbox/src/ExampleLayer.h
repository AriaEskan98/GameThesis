#pragma once

#include "Hazel.h"

class ExampleLayer : public Hazel::Layer
{
public:
	ExampleLayer();
	virtual ~ExampleLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(Hazel::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(Hazel::Event& e) override;
private:
	Hazel::ShaderLibrary myShaderLibrary;
	Hazel::Handle<Hazel::Shader> myShader;
	Hazel::Handle<Hazel::VertexArray> myVertexArray;

	Hazel::Handle<Hazel::Shader> myFlatColorShader;
	Hazel::Handle<Hazel::VertexArray> mySquareVA;

	Hazel::Handle<Hazel::Texture2D> myTexture, myChernoLogoTexture;

	Hazel::OrthographicCameraController myCameraController;
	glm::vec3 mySquareColor = { 0.2f, 0.3f, 0.8f };
};
