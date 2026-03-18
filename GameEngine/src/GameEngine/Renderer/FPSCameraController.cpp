#include "gepch.h"
#include "GameEngine/Renderer/FPSCameraController.h"

#include "GameEngine/Core/Application.h"
#include "GameEngine/Core/Input.h"
#include "GameEngine/Core/KeyCodes.h"
#include "GameEngine/Core/MouseCodes.h"
#include "GameEngine/Events/KeyEvent.h"

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

		// Horizontal movement — ignore the Y component of forward so the player
		// moves in the ground plane regardless of pitch.
		glm::vec3 flatForward = glm::normalize(glm::vec3(myForward.x, 0.0f, myForward.z));

		glm::vec3 horizontal = { 0.0f, 0.0f, 0.0f };
		if (Input::IsKeyPressed(Key::W)) horizontal += flatForward * speed;
		if (Input::IsKeyPressed(Key::S)) horizontal -= flatForward * speed;
		if (Input::IsKeyPressed(Key::A)) horizontal -= myRight     * speed;
		if (Input::IsKeyPressed(Key::D)) horizontal += myRight     * speed;

		if (myPhysicsBody)
		{
			// ---- Physics-driven path ----------------------------------------
			// Read last frame's position from the physics body (eye = feet + EyeHeight).
			myPosition = myPhysicsBody->Position + glm::vec3(0.0f, EyeHeight, 0.0f);
			myIsGrounded = myPhysicsBody->IsGrounded;

			// Jump: apply an upward velocity impulse when grounded.
			float verticalVel = myPhysicsBody->Velocity.y;
			if (Input::IsKeyPressed(Key::Space) && myIsGrounded)
				verticalVel = JumpSpeed;

			// Write the desired velocity back so the physics world can resolve collisions.
			myPhysicsBody->Velocity = { horizontal.x, verticalVel, horizontal.z };
		}
		else
		{
			// ---- Standalone gravity simulation ------------------------------
			// Apply gravity every frame unless standing on the implicit floor.
			if (!myIsGrounded)
				myVerticalVelocity += GravityAccel * dt;

			myPosition += horizontal * dt;
			myPosition.y += myVerticalVelocity * dt;

			// Implicit floor at EyeHeight (y = 0 is ground level).
			if (myPosition.y <= EyeHeight)
			{
				myPosition.y    = EyeHeight;
				myVerticalVelocity = 0.0f;
				myIsGrounded    = true;
			}
			else
			{
				myIsGrounded = false;
			}

			// Jump from the floor.
			if (Input::IsKeyPressed(Key::Space) && myIsGrounded)
			{
				myVerticalVelocity = JumpSpeed;
				myIsGrounded       = false;
			}
		}

		UpdateView();
	}

	void FPSCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseMovedEvent>([this](MouseMovedEvent& e) { return OnMouseMoved(e); });
		dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return OnWindowResized(e); });
		dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) { return OnKeyPressed(e); });
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
