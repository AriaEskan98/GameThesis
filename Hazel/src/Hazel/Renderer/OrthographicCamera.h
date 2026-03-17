#pragma once

#include <glm/glm.hpp>

namespace GameEngine {

	class OrthographicCamera
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		void SetProjection(float left, float right, float bottom, float top);

		const glm::vec3& GetPosition() const { return myPosition; }
		void SetPosition(const glm::vec3& position) { myPosition = position; RecalculateViewMatrix(); }

		float GetRotation() const { return myRotation; }
		void SetRotation(float rotation) { myRotation = rotation; RecalculateViewMatrix(); }

		const glm::mat4& GetProjectionMatrix() const { return myProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return myViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return myViewProjectionMatrix; }
	private:
		void RecalculateViewMatrix();
	private:
		glm::mat4 myProjectionMatrix;
		glm::mat4 myViewMatrix;
		glm::mat4 myViewProjectionMatrix;

		glm::vec3 myPosition = { 0.0f, 0.0f, 0.0f };
		float myRotation = 0.0f;
	};

}
