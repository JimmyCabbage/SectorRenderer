#ifndef CAMERA_HPP_H
#define CAMERA_HPP_H

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace cam
{
	enum class Movement : unsigned int
	{
		Forward = 1 << 1,
		Backward = 1 << 2,
		Left = 1 << 3,
		Right = 1 << 4
	};

	class Camera
	{
		void updateCameraVectors();

	protected:
		glm::vec3 Position, Front, Up, Right;
		const glm::vec3 WorldUp;

		float Yaw, Pitch;

	public:
		explicit Camera(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f);

		glm::mat4 GetViewMatrix() const;

		void ProcessKeyboard(const Movement dir, const float deltaTime);

		void ProcessMouseMovement(float xoffset, float yoffset);
	};
}

#endif