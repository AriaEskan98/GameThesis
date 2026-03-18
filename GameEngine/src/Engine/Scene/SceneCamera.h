#pragma once

#include "Engine/Renderer/Camera.h"

namespace GameEngine {

	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };
	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		void SetPerspective(float verticalFOV, float nearClip, float farClip);
		void SetOrthographic(float size, float nearClip, float farClip);

		void SetViewportSize(uint32_t width, uint32_t height);

		float GetPerspectiveVerticalFOV() const { return myPerspectiveFOV; }
		void SetPerspectiveVerticalFOV(float verticalFov) { myPerspectiveFOV = verticalFov; RecalculateProjection(); }
		float GetPerspectiveNearClip() const { return myPerspectiveNear; }
		void SetPerspectiveNearClip(float nearClip) { myPerspectiveNear = nearClip; RecalculateProjection(); }
		float GetPerspectiveFarClip() const { return myPerspectiveFar; }
		void SetPerspectiveFarClip(float farClip) { myPerspectiveFar = farClip; RecalculateProjection(); }

		float GetOrthographicSize() const { return myOrthographicSize; }
		void SetOrthographicSize(float size) { myOrthographicSize = size; RecalculateProjection(); }
		float GetOrthographicNearClip() const { return myOrthographicNear; }
		void SetOrthographicNearClip(float nearClip) { myOrthographicNear = nearClip; RecalculateProjection(); }
		float GetOrthographicFarClip() const { return myOrthographicFar; }
		void SetOrthographicFarClip(float farClip) { myOrthographicFar = farClip; RecalculateProjection(); }

		ProjectionType GetProjectionType() const { return myProjectionType; }
		void SetProjectionType(ProjectionType type) { myProjectionType = type; RecalculateProjection(); }
	private:
		void RecalculateProjection();
	private:
		ProjectionType myProjectionType = ProjectionType::Orthographic;

		float myPerspectiveFOV = glm::radians(45.0f);
		float myPerspectiveNear = 0.01f, myPerspectiveFar = 1000.0f;

		float myOrthographicSize = 10.0f;
		float myOrthographicNear = -1.0f, myOrthographicFar = 1.0f;

		float myAspectRatio = 0.0f;
	};

}
