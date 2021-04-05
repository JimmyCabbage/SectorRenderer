#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <vector>

#include <glm/glm.hpp>

#include "Sector.hpp"

constexpr float EYE_HEIGHT = 6.0f;

class Player
{
	void update_vectors()
	{
		front = glm::normalize(glm::vec3
		{
			cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
			sin(glm::radians(pitch)),
			sin(glm::radians(yaw)) * cos(glm::radians(pitch))
		});

		front2d = glm::normalize(glm::vec2
			{
				cos(glm::radians(yaw)),
				sin(glm::radians(yaw))
			});

		const glm::vec3 temp_right = glm::normalize(glm::cross(glm::vec3{ front2d.x, 0.0f, front2d.y }, world_up));

		right2d = glm::vec2{ temp_right.x, temp_right.z };
	}

	glm::vec3 position, velocity;
	glm::vec3 front;
	glm::vec2 front2d, right2d;
	bool falling = false;
	float y_add;
	const glm::vec3 world_up;

	uint32_t sector;

	float yaw, pitch;

public:
	explicit Player(uint32_t sector = 0, glm::vec3 pos = glm::vec3{ 0.0f }, glm::vec3 world_up = glm::vec3{ 0.0f, 1.0f, 0.0f }, float yaw = -90.0f, float pitch = 0.0f)
		: sector(sector), position(pos), velocity(glm::vec3{ 0.0f }), front(glm::vec3{ 0.0f }), world_up(world_up), yaw(yaw), pitch(pitch)
	{
		update_vectors();
	}

	Player& operator=(Player&& player) noexcept
	{
		if (&player == this)
		{
			return *this;
		}

		position = std::move(player.position);
		velocity = std::move(player.velocity);
		front = std::move(player.front);
		sector = std::move(player.sector);
		pitch = std::move(player.pitch);
		yaw = std::move(player.yaw);

		update_vectors();

		return *this;
	}

	glm::mat4 get_view_matrix() const;

	enum class MoveDir
	{
		NONE = 0,
		FORWARD = 1<<1,
		BACKWARD = 1<<2,
		LEFT = 1<<3,
		RIGHT = 1<<4
	};

	void move(MoveDir dir, const double deltatime);

	void collision(const std::vector<Sector>& sectors, const double deltatime);

	void mouse_move(float xoffset, float yoffset)
	{
		xoffset *= 0.1f;
		yoffset *= 0.1f;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f)
		{
			pitch = 89.0f;
		}
		else if (pitch < -89.0f)
		{
			pitch = -89.0f;
		}

		yaw = fmod((yaw + xoffset), 360.0f);

		update_vectors();
	}
};

static Player::MoveDir operator&(Player::MoveDir a, Player::MoveDir b)
{
	return static_cast<Player::MoveDir>(static_cast<int>(a) & static_cast<int>(b));
}

static Player::MoveDir operator|(Player::MoveDir a, Player::MoveDir b)
{
	return static_cast<Player::MoveDir>(static_cast<int>(a) | static_cast<int>(b));
}

#endif