#pragma once

#include "Camera.h"
#include "Engine/Core/Timestep.h"
#include "Engine/Events/Event.h"
#include "Engine/Events/MouseEvent.h"

#include <glm/glm.hpp>

namespace GameEngine {

	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		inline float GetDistance() const { return myDistance; }
		inline void SetDistance(float distance) { myDistance = distance; }

		inline void SetViewportSize(float width, float height) { myViewportWidth = width; myViewportHeight = height; UpdateProjection(); }

		const glm::mat4& GetViewMatrix() const { return myViewMatrix; }
		glm::mat4 GetViewProjection() const { return myProjection * myViewMatrix; }

		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		const glm::vec3& GetPosition() const { return myPosition; }
		glm::quat GetOrientation() const;

		float GetPitch() const { return myPitch; }
		float GetYaw() const { return myYaw; }
	private:
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;
	private:
		float myFOV = 45.0f, myAspectRatio = 1.778f, myNearClip = 0.1f, myFarClip = 1000.0f;

		glm::mat4 myViewMatrix;
		glm::vec3 myPosition = { 0.0f, 0.0f, 0.0f };
		glm::vec3 myFocalPoint = { 0.0f, 0.0f, 0.0f };

		glm::vec2 myInitialMousePosition = { 0.0f, 0.0f };

		float myDistance = 10.0f;
		float myPitch = 0.0f, myYaw = 0.0f;

		float myViewportWidth = 1280, myViewportHeight = 720;
	};

}
