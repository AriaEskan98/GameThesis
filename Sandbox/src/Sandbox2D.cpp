#include "Sandbox2D.h"
#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), myCameraController(1280.0f / 720.0f)
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
		GameEngine::RenderCommand::SetClearColor(myBackgroundColor);
		GameEngine::RenderCommand::Clear();
	}

	{
		static float rotation = 0.0f;
		rotation += ts * 50.0f;

		GE_PROFILE_SCOPE("Renderer Draw");
		GameEngine::Renderer2D::BeginScene(myCameraController.GetCamera());
		GameEngine::Renderer2D::DrawRotatedQuad({ 1.0f, 0.0f }, { 0.8f, 0.8f }, -45.0f, mySquareColors[0]);
		GameEngine::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, mySquareColors[1]);
		GameEngine::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, mySquareColors[2]);
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
	ImGui::ColorEdit4("Background Color", glm::value_ptr(myBackgroundColor));

	auto stats = GameEngine::Renderer2D::GetStats();
	ImGui::Text("Renderer2D Stats:");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quads: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

	if (mySelectedSquare >= 0)
	{
		const char* names[] = { "Rotated Square Color", "Flat Square Color", "Blue Square Color" };
		ImGui::ColorEdit4(names[mySelectedSquare], glm::value_ptr(mySquareColors[mySelectedSquare]));
	}
	else
	{
		ImGui::Text("Click a square to select it");
	}
	ImGui::End();
}

void Sandbox2D::OnEvent(GameEngine::Event& e)
{
	myCameraController.OnEvent(e);

	GameEngine::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<GameEngine::MouseButtonPressedEvent>(GE_BIND_FN(OnMouseButtonPressed));
}

bool Sandbox2D::OnMouseButtonPressed(GameEngine::MouseButtonPressedEvent& e)
{
	if (e.GetMouseButton() != GameEngine::Mouse::ButtonLeft)
		return false;

	if (ImGui::GetIO().WantCaptureMouse)
		return false;

	// Convert mouse position to world space
	glm::vec2 mousePos = GameEngine::Input::GetMousePosition();
	auto& window = GameEngine::Application::GetInstance().GetWindow();
	float ndcX = (mousePos.x / (float)window.GetWidth())  * 2.0f - 1.0f;
	float ndcY = 1.0f - (mousePos.y / (float)window.GetHeight()) * 2.0f;

	glm::mat4 invVP = glm::inverse(myCameraController.GetCamera().GetViewProjectionMatrix());
	glm::vec4 worldPos = invVP * glm::vec4(ndcX, ndcY, 0.0f, 1.0f);
	glm::vec2 mouse(worldPos.x, worldPos.y);

	// Hit test quad 2: center (0.5, -0.5), half-size (0.25, 0.375)
	if (mouse.x >= 0.25f && mouse.x <= 0.75f && mouse.y >= -0.875f && mouse.y <= 0.125f)
	{
		mySelectedSquare = 2;
		return true;
	}

	// Hit test quad 1: center (-1.0, 0.0), half-size (0.4, 0.4)
	if (mouse.x >= -1.4f && mouse.x <= -0.6f && mouse.y >= -0.4f && mouse.y <= 0.4f)
	{
		mySelectedSquare = 1;
		return true;
	}

	// Hit test quad 0 (rotated -45°): center (1.0, 0.0), half-size (0.4, 0.4)
	// Transform mouse to local space by rotating +45° around center
	glm::vec2 local = mouse - glm::vec2(1.0f, 0.0f);
	float cosA = glm::cos(glm::radians(45.0f));
	float sinA = glm::sin(glm::radians(45.0f));
	glm::vec2 localRot(local.x * cosA - local.y * sinA, local.x * sinA + local.y * cosA);
	if (localRot.x >= -0.4f && localRot.x <= 0.4f && localRot.y >= -0.4f && localRot.y <= 0.4f)
	{
		mySelectedSquare = 0;
		return true;
	}

	mySelectedSquare = -1;
	return false;
}
