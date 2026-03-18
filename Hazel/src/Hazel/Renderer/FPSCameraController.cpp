#include "gepch.h"
#include "Hazel/Renderer/FPSCameraController.h"

#include "Hazel/Core/Application.h"
#include "Hazel/Core/Input.h"
#include "Hazel/Core/KeyCodes.h"
#include "Hazel/Core/MouseCodes.h"
#include "Hazel/Events/KeyEvent.h"

#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace GameEngine {

	FPSCameraController::FPSCameraController(float fov, float aspectRatio,
	                                         float nearClip, float farClip)
		: myFOV(fov), myAspectRatio(aspectRatio),
		  myNearClip(nearClip), myFarClip(farClip)
	{
		UpdateProjection();
		UpdateView();
	}

	// -------------------------------------------------------------------------

	void FPSCameraController::UpdateProjection()
	{
		myCamera = Camera(glm::perspective(
			glm::radians(myFOV), myAspectRatio, myNearClip, myFarClip));
	}

	void FPSCameraController::UpdateView()
	{
		// Recompute forward vector from yaw and pitch.
		glm::vec3 front;
		front.x = glm::cos(glm::radians(myYaw)) * glm::cos(glm::radians(myPitch));
		front.y = glm::sin(glm::radians(myPitch));
		front.z = glm::sin(glm::radians(myYaw)) * glm::cos(glm::radians(myPitch));

		myForward = glm::normalize(front);
		myRight   = glm::normalize(glm::cross(myForward, glm::vec3(0.0f, 1.0f, 0.0f)));

		myViewMatrix = glm::lookAt(myPosition, myPosition + myForward, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	// -------------------------------------------------------------------------

	void FPSCameraController::SetAspectRatio(float ratio)
	{
		myAspectRatio = ratio;
		UpdateProjection();
	}

	void FPSCameraController::SetFPSMode(bool enabled)
	{
		if (myFPSMode == enabled)
			return;

		myFPSMode  = enabled;
		myFirstMouse = true; // Discard the first delta after mode switch.

		auto& window = Application::GetInstance().GetWindow();
		window.SetCursorMode(enabled ? CursorMode::Locked : CursorMode::Normal);
	}

	// -------------------------------------------------------------------------

	void FPSCameraController::OnUpdate(Timestep ts)
	{
		if (!myFPSMode)
			return;

		const float speed = Input::IsKeyPressed(Key::LeftShift)
		                    ? myMovementSpeed * 2.0f
		                    : myMovementSpeed;
		const float dt    = ts;

		// Move in the horizontal plane (ignore the Y component of forward so the
		// player does not drift up or down while looking at the sky or ground).
		glm::vec3 flatForward = glm::normalize(glm::vec3(myForward.x, 0.0f, myForward.z));

		if (Input::IsKeyPressed(Key::W))          myPosition += flatForward * speed * dt;
		if (Input::IsKeyPressed(Key::S))          myPosition -= flatForward * speed * dt;
		if (Input::IsKeyPressed(Key::A))          myPosition -= myRight     * speed * dt;
		if (Input::IsKeyPressed(Key::D))          myPosition += myRight     * speed * dt;
		if (Input::IsKeyPressed(Key::Space))      myPosition.y += speed * dt;
		if (Input::IsKeyPressed(Key::LeftControl))myPosition.y -= speed * dt;

		UpdateView();
	}

	void FPSCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseMovedEvent>     (GE_BIND_FN(OnMouseMoved));
		dispatcher.Dispatch<WindowResizeEvent>   (GE_BIND_FN(OnWindowResized));
		dispatcher.Dispatch<KeyPressedEvent>     (GE_BIND_FN(OnKeyPressed));
	}

	// -------------------------------------------------------------------------

	bool FPSCameraController::OnMouseMoved(MouseMovedEvent& e)
	{
		if (!myFPSMode)
			return false;

		float x = e.GetX();
		float y = e.GetY();

		if (myFirstMouse)
		{
			myLastMouseX = x;
			myLastMouseY = y;
			myFirstMouse = false;
			return false;
		}

		float dx = (x - myLastMouseX) * myMouseSensitivity;
		float dy = (myLastMouseY - y) * myMouseSensitivity; // Y is inverted in screen space.

		myLastMouseX = x;
		myLastMouseY = y;

		myYaw   += dx;
		myPitch += dy;

		// Clamp pitch to prevent the camera from flipping over.
		myPitch = glm::clamp(myPitch, -89.0f, 89.0f);

		UpdateView();
		return false; // Allow other handlers to see the event.
	}

	bool FPSCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		if (e.GetHeight() == 0)
			return false;

		SetAspectRatio((float)e.GetWidth() / (float)e.GetHeight());
		return false;
	}

	bool FPSCameraController::OnKeyPressed(KeyPressedEvent& e)
	{
		// Escape exits FPS mode so the player can access menus.
		if (e.GetKeyCode() == Key::Escape && myFPSMode)
		{
			SetFPSMode(false);
			return true;
		}
		return false;
	}

}
