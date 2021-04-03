#ifndef PLAYER_HPP
#define PLAYER_HPP

//header headers
#include <glm/glm.hpp>

//cpp headers
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

class Player
{
	void update_vectors()
	{
		front = glm::vec3
		{
			cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
			sin(glm::radians(pitch)),
			sin(glm::radians(yaw)) * cos(glm::radians(pitch))
		};

		front = glm::normalize(front);

		right = glm::normalize(glm::cross(front, world_up));
	}

	glm::vec3 position, velocity;
	glm::vec3 front, right;
	const glm::vec3 world_up;

	float yaw, pitch;

public:
	explicit Player(glm::vec3 pos = glm::vec3{ 0.0f }, glm::vec3 world_up = glm::vec3{ 0.0f, 1.0f, 0.0f }, float yaw = -90.0f, float pitch = 0.0f)
		: position(pos), velocity(glm::vec3{ 0.0f }), front(glm::vec3{ 0.0f }), world_up(world_up), yaw(yaw), pitch(pitch)
	{
		update_vectors();
	}

	glm::mat4 get_view_matrix() const
	{
		return glm::lookAt(position, position + front, world_up);
	}
};

#endif