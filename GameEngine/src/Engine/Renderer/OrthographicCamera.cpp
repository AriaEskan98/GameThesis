#include "gepch.h"
#include "Engine/Renderer/OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace GameEngine {

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		: myProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), myViewMatrix(1.0f)
	{
		GE_PROFILE_FUNCTION();

		myViewProjectionMatrix = myProjectionMatrix * myViewMatrix;
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
	{
		GE_PROFILE_FUNCTION();

		myProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
		myViewProjectionMatrix = myProjectionMatrix * myViewMatrix;
	}

	void OrthographicCamera::RecalculateViewMatrix()
	{
		GE_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), myPosition) *
			glm::rotate(glm::mat4(1.0f), glm::radians(myRotation), glm::vec3(0, 0, 1));

		myViewMatrix = glm::inverse(transform);
		myViewProjectionMatrix = myProjectionMatrix * myViewMatrix;
	}

}