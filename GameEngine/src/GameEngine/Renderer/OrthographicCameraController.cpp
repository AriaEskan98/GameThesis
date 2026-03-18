#include "gepch.h"
#include "GameEngine/Renderer/OrthographicCameraController.h"

#include "GameEngine/Core/Input.h"
#include "GameEngine/Core/KeyCodes.h"

namespace GameEngine {

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: myAspectRatio(aspectRatio), myCamera(-myAspectRatio * myZoomLevel, myAspectRatio * myZoomLevel, -myZoomLevel, myZoomLevel), myRotation(rotation)
	{
	}

	void OrthographicCameraController::OnUpdate(Timestep ts)
	{
		GE_PROFILE_FUNCTION();

		if (Input::IsKeyPressed(Key::A))
		{
			myCameraPosition.x -= cos(glm::radians(myCameraRotation)) * myCameraTranslationSpeed * ts;
			myCameraPosition.y -= sin(glm::radians(myCameraRotation)) * myCameraTranslationSpeed * ts;
		}
		else if (Input::IsKeyPressed(Key::D))
		{
			myCameraPosition.x += cos(glm::radians(myCameraRotation)) * myCameraTranslationSpeed * ts;
			myCameraPosition.y += sin(glm::radians(myCameraRotation)) * myCameraTranslationSpeed * ts;
		}

		if (Input::IsKeyPressed(Key::W))
		{
			myCameraPosition.x += -sin(glm::radians(myCameraRotation)) * myCameraTranslationSpeed * ts;
			myCameraPosition.y += cos(glm::radians(myCameraRotation)) * myCameraTranslationSpeed * ts;
		}
		else if (Input::IsKeyPressed(Key::S))
		{
			myCameraPosition.x -= -sin(glm::radians(myCameraRotation)) * myCameraTranslationSpeed * ts;
			myCameraPosition.y -= cos(glm::radians(myCameraRotation)) * myCameraTranslationSpeed * ts;
		}

		if (myRotation)
		{
			if (Input::IsKeyPressed(Key::Q))
				myCameraRotation += myCameraRotationSpeed * ts;
			if (Input::IsKeyPressed(Key::E))
				myCameraRotation -= myCameraRotationSpeed * ts;

			if (myCameraRotation > 180.0f)
				myCameraRotation -= 360.0f;
			else if (myCameraRotation <= -180.0f)
				myCameraRotation += 360.0f;

			myCamera.SetRotation(myCameraRotation);
		}

		myCamera.SetPosition(myCameraPosition);

		myCameraTranslationSpeed = myZoomLevel;
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		GE_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(GE_BIND_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(GE_BIND_FN(OrthographicCameraController::OnWindowResized));
	}

	void OrthographicCameraController::OnResize(float width, float height)
	{
		myAspectRatio = width / height;
		myCamera.SetProjection(-myAspectRatio * myZoomLevel, myAspectRatio * myZoomLevel, -myZoomLevel, myZoomLevel);
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		GE_PROFILE_FUNCTION();

		myZoomLevel -= e.GetYOffset() * 0.25f;
		myZoomLevel = std::max(myZoomLevel, 0.25f);
		myCamera.SetProjection(-myAspectRatio * myZoomLevel, myAspectRatio * myZoomLevel, -myZoomLevel, myZoomLevel);
		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		GE_PROFILE_FUNCTION();

		OnResize((float)e.GetWidth(), (float)e.GetHeight());
		return false;
	}

}