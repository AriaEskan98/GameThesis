#pragma once

#include "Hazel/Core/Timestep.h"
#include "Hazel/Core/Window.h"
#include "Hazel/Events/Event.h"
#include "Hazel/Events/MouseEvent.h"
#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Renderer/Camera.h"

#include <glm/glm.hpp>

namespace GameEngine {

	/// First-person camera controller.
	///
	/// Wraps a perspective Camera and tracks position, yaw and pitch independently
	/// so the player moves in the horizontal plane regardless of where they are
	/// looking (standard FPS behaviour).
	///
	/// Usage:
	///   - Call SetFPSMode(true) to lock the cursor and enter game mode.
	///   - Call OnUpdate(ts) every frame from your layer's OnUpdate.
	///   - Call OnEvent(e) from your layer's OnEvent.
	///   - Pass GetViewProjection() / GetPosition() to Renderer3D::BeginScene.
	///
	/// Key bindings (non-configurable defaults):
	///   W/S   — forward / backward
	///   A/D   — strafe left / right
	///   Space — move up
	///   Ctrl  — move down
	///   Shift — sprint (2× speed)
	///   Escape — exit FPS mode
	class FPSCameraController
	{
	public:
		/// @param fov         Vertical field of view in degrees.
		/// @param aspectRatio Width / height.
		/// @param nearClip    Near clip plane distance.
		/// @param farClip     Far clip plane distance.
		FPSCameraController(float fov = 60.0f, float aspectRatio = 1.778f,
		                    float nearClip = 0.1f, float farClip = 1000.0f);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		// ---- Rendering accessors ----------------------------------------

		const Camera&      GetCamera()          const { return myCamera; }
		glm::mat4          GetViewMatrix()       const { return myViewMatrix; }
		glm::mat4          GetViewProjection()   const { return myCamera.GetProjection() * myViewMatrix; }
		const glm::vec3&   GetPosition()         const { return myPosition; }

		// ---- Configuration ----------------------------------------------

		/// Enter or leave FPS mode. In FPS mode the cursor is locked and hidden;
		/// mouse movement is always applied to the view direction.
		void SetFPSMode(bool enabled);
		bool IsFPSMode() const { return myFPSMode; }

		void SetPosition(const glm::vec3& pos) { myPosition = pos; UpdateView(); }
		void SetYaw(float yaw)     { myYaw   = yaw;   UpdateView(); }
		void SetPitch(float pitch) { myPitch = pitch; UpdateView(); }

		void SetMovementSpeed(float speed)     { myMovementSpeed    = speed; }
		void SetMouseSensitivity(float sens)   { myMouseSensitivity = sens;  }
		void SetAspectRatio(float ratio);

	private:
		void UpdateProjection();
		void UpdateView();

		bool OnMouseMoved(MouseMovedEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);
		bool OnKeyPressed(KeyPressedEvent& e);

	private:
		Camera    myCamera;
		glm::mat4 myViewMatrix = glm::mat4(1.0f);

		glm::vec3 myPosition = { 0.0f, 1.75f, 3.0f }; ///< Eye position (≈ standing height).
		float myYaw   = -90.0f; ///< Degrees; -90 = looking toward -Z.
		float myPitch =   0.0f; ///< Degrees; clamped to ±89°.

		glm::vec3 myForward = { 0.0f, 0.0f, -1.0f };
		glm::vec3 myRight   = { 1.0f, 0.0f,  0.0f };

		float myFOV, myAspectRatio, myNearClip, myFarClip;
		float myMovementSpeed    = 5.0f;
		float myMouseSensitivity = 0.1f;

		bool  myFPSMode     = false;
		float myLastMouseX  = 0.0f;
		float myLastMouseY  = 0.0f;
		bool  myFirstMouse  = true;
	};

}
