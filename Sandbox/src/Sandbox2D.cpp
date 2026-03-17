#include "Sandbox2D.h"
#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), myCameraController(1280.0f / 720.0f), mySquareColor({ 0.2f, 0.3f, 0.8f, 1.0f })
{
}

void Sandbox2D::OnAttach()
{
	GE_PROFILE_FUNCTION();

	myCheckerboardTexture = GameEngine::Texture2D::Create("assets/textures/Checkerboard.png");
}

void Sandbox2D::OnDetach()
{
	GE_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(GameEngine::Timestep ts)
{
	GE_PROFILE_FUNCTION();

	// Update
	myCameraController.OnUpdate(ts);

	// Render
	GameEngine::Renderer2D::ResetStats();
	{
		GE_PROFILE_SCOPE("Renderer Prep");
		GameEngine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		GameEngine::RenderCommand::Clear();
	}

	{
		static float rotation = 0.0f;
		rotation += ts * 50.0f;

		GE_PROFILE_SCOPE("Renderer Draw");
		GameEngine::Renderer2D::BeginScene(myCameraController.GetCamera());
		GameEngine::Renderer2D::DrawRotatedQuad({ 1.0f, 0.0f }, { 0.8f, 0.8f }, -45.0f, { 0.8f, 0.2f, 0.3f, 1.0f });
		GameEngine::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, { 0.8f, 0.2f, 0.3f, 1.0f });
		GameEngine::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, mySquareColor);
		GameEngine::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 20.0f, 20.0f }, myCheckerboardTexture, 10.0f);
		GameEngine::Renderer2D::DrawRotatedQuad({ -2.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, rotation, myCheckerboardTexture, 20.0f);
		GameEngine::Renderer2D::EndScene();

		GameEngine::Renderer2D::BeginScene(myCameraController.GetCamera());
		for (float y = -5.0f; y < 5.0f; y += 0.5f)
		{
			for (float x = -5.0f; x < 5.0f; x += 0.5f)
			{
				glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
				GameEngine::Renderer2D::DrawQuad({ x, y }, { 0.45f, 0.45f }, color);
			}
		}
		GameEngine::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	GE_PROFILE_FUNCTION();

	ImGui::Begin("Settings");

	auto stats = GameEngine::Renderer2D::GetStats();
	ImGui::Text("Renderer2D Stats:");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quads: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

	ImGui::ColorEdit4("Square Color", glm::value_ptr(mySquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(GameEngine::Event& e)
{
	myCameraController.OnEvent(e);
}
