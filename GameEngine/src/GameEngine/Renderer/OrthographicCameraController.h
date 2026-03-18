#pragma once

#include "GameEngine/Renderer/OrthographicCamera.h"
#include "GameEngine/Core/Timestep.h"

#include "GameEngine/Events/ApplicationEvent.h"
#include "GameEngine/Events/MouseEvent.h"

namespace GameEngine {

	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		void OnResize(float width, float height);

		OrthographicCamera& GetCamera() { return myCamera; }
		const OrthographicCamera& GetCamera() const { return myCamera; }

		float GetZoomLevel() const { return myZoomLevel; }
		void SetZoomLevel(float level) { myZoomLevel = level; }
	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);
	private:
		float myAspectRatio;
		float myZoomLevel = 1.0f;
		OrthographicCamera myCamera;

		bool myRotation;

		glm::vec3 myCameraPosition = { 0.0f, 0.0f, 0.0f };
		float myCameraRotation = 0.0f; //In degrees, in the anti-clockwise direction
		float myCameraTranslationSpeed = 5.0f, myCameraRotationSpeed = 180.0f;
	};

}