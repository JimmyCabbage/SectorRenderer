#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <vector>

#include <glm/glm.hpp>

#include "Sector.hpp"

class Player
{
	void update_vectors();

	glm::vec3 position, velocity;
	glm::vec3 front;
	glm::vec2 front2d, right2d;
	bool falling = false;
	const glm::vec3 world_up;

	uint32_t sector;

	float yaw, pitch;

public:
	explicit Player(uint32_t sector = 0, glm::vec3 pos = glm::vec3{ 0.0f }, glm::vec3 world_up = glm::vec3{ 0.0f, 1.0f, 0.0f }, float yaw = -90.0f, float pitch = 0.0f);

	Player& operator=(Player&& player) noexcept;

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

	void mouse_move(float xoffset, float yoffset);

	constexpr static float get_eye_height()
	{
		return 6.0f;
	}
};

Player::MoveDir operator&(Player::MoveDir a, Player::MoveDir b);

Player::MoveDir operator|(Player::MoveDir a, Player::MoveDir b);

#endif