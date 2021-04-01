#include "Camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

namespace cam
{
	inline void Camera::updateCameraVectors()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);

		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up = glm::normalize(glm::cross(Right, Front));
	}

	Camera::Camera(glm::vec3 pos, glm::vec3 worldUp, float yaw, float pitch)
		: Position(pos), Front(glm::vec3(0.0f, 0.0f, -1.0f)), WorldUp(worldUp), Yaw(yaw), Pitch(pitch)
	{
		updateCameraVectors();
	}

	glm::mat4 Camera::GetViewMatrix() const
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	void Camera::ProcessKeyboard(const Movement dir, const float deltaTime, const bool sprinting)
	{
		float velocity = 0.0f;
		if (sprinting)
		{
			velocity = 25.0f * deltaTime;
		}
		else
		{
			velocity = 2.5f * deltaTime;
		}

		if ((dir == Movement::Forward))
		{
			Position += Front * velocity;
		}
		else if ((dir == Movement::Backward))
		{
			Position -= Front * velocity;
		}
		else if ((dir == Movement::Right))
		{
			Position += Right * velocity;
		}
		else if ((dir == Movement::Left))
		{
			Position -= Right * velocity;
		}
	}

	void Camera::ProcessMouseMovement(float xoffset, float yoffset)
	{
		xoffset *= 0.1f;
		yoffset *= 0.1f;

		Yaw += xoffset;
		Pitch += yoffset;

		if (Pitch > 89.0f)
		{
			Pitch = 89.0f;
		}
		else if (Pitch < -89.0f)
		{
			Pitch = -89.0f;
		}

		Yaw = fmod((Yaw + xoffset), 360.0f);

		updateCameraVectors();
	}

	void Camera::Reset() noexcept
	{
		Position = {};
		Pitch = {};
		Yaw = {};
	}
}