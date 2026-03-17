#include "gepch.h"
#include "SceneCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace GameEngine {

	SceneCamera::SceneCamera()
	{
		RecalculateProjection();
	}

	void SceneCamera::SetPerspective(float verticalFOV, float nearClip, float farClip)
	{
		myProjectionType = ProjectionType::Perspective;
		myPerspectiveFOV = verticalFOV;
		myPerspectiveNear = nearClip;
		myPerspectiveFar = farClip;
		RecalculateProjection();
	}

	void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
	{
		myProjectionType = ProjectionType::Orthographic;
		myOrthographicSize = size;
		myOrthographicNear = nearClip;
		myOrthographicFar = farClip;
		RecalculateProjection();
	}

	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		GE_CORE_ASSERT(width > 0 && height > 0);
		myAspectRatio = (float)width / (float)height;
		RecalculateProjection();
	}

	void SceneCamera::RecalculateProjection()
	{
		if (myProjectionType == ProjectionType::Perspective)
		{
			myProjection = glm::perspective(myPerspectiveFOV, myAspectRatio, myPerspectiveNear, myPerspectiveFar);
		}
		else
		{
			float orthoLeft = -myOrthographicSize * myAspectRatio * 0.5f;
			float orthoRight = myOrthographicSize * myAspectRatio * 0.5f;
			float orthoBottom = -myOrthographicSize * 0.5f;
			float orthoTop = myOrthographicSize * 0.5f;

			myProjection = glm::ortho(orthoLeft, orthoRight,
				orthoBottom, orthoTop, myOrthographicNear, myOrthographicFar);
		}
		
	}

}
