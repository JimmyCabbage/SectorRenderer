#include "Player.hpp"

#include <cmath>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

void Player::update_vectors()
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

Player::Player(uint32_t sector, glm::vec3 pos, glm::vec3 world_up, float yaw, float pitch)
	: position(pos), velocity(glm::vec3{ 0.0f }), front(glm::vec3{ 0.0f }), world_up(world_up), sector(sector), yaw(yaw), pitch(pitch)
{
	update_vectors();
}

Player& Player::operator=(Player&& player) noexcept
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

glm::mat4 Player::get_view_matrix() const
{
	return glm::lookAt(position, position + front, world_up);
}

void Player::move(MoveDir dir, const double deltatime)
{
	const float speed = 12.5f * static_cast<float>(deltatime);

	glm::vec2 move_dir{ 0.0f, 0.0f };

	if ((dir & MoveDir::FORWARD) == MoveDir::FORWARD)
	{
		//moving forward is slightly faster
		move_dir += front2d * (speed + speed * 0.5f);
	}
	if ((dir & MoveDir::BACKWARD) == MoveDir::BACKWARD)
	{
		move_dir -= front2d * speed;
	}
	if ((dir & MoveDir::LEFT) == MoveDir::LEFT)
	{
		move_dir -= right2d * speed;
	}
	if ((dir & MoveDir::RIGHT) == MoveDir::RIGHT)
	{
		move_dir += right2d * speed;
	}

	velocity.x = velocity.x * (1 - 0.2f) + move_dir.x * 0.2f;
	velocity.z = velocity.z * (1 - 0.2f) + move_dir.y * 0.2f;
}

void Player::collision(const std::vector<Sector>& sectors, const double deltatime)
{
	const auto& sect = sectors[sector];

	//horizontol check
	if (falling) velocity.y = velocity.y * 0.95f + (-25.0f * static_cast<float>(deltatime)) * 0.05f;

	const float nextz = position.y + velocity.y;
	if (velocity.y < 0 && nextz < sectors[sector].floor + get_eye_height())
	{
		position.y = sectors[sector].floor + get_eye_height();
		velocity.y = 0;
		falling = false;
	}
	else if (velocity.y > 0 && nextz > sectors[sector].ceil - 0.5f)
	{
		velocity.y = 0.0f;
		falling = true;
	}

	position.y += velocity.y;

	//vertical check
	for (size_t i = 0; i < sect.vertices.size(); i++)
	{
		const auto& vert1 = sect.vertices[i];

		size_t ref = 0;
		if (i == sect.vertices.size() - 1)
		{
			ref = 0;
		}
		else
		{
			ref = i + 1;
		}
		const auto& vert2 = sect.vertices[ref];

#define vxs(x0,y0, x1,y1)	((x0)*(y1) - (x1)*(y0))   // vxs: Vector cross product
#define PointSide(px,py, x0,y0, x1,y1) vxs((x1)-(x0), (y1)-(y0), (px)-(x0), (py)-(y0))

		const float side = PointSide(position.x + velocity.x, position.z + velocity.z, vert1.x, vert1.y, vert2.x, vert2.y);

		if (side > 0.0f)
		{
			const float hole_low = sect.neighbors[i] < 0 ? 15e15f : std::max(sect.floor, sectors[sect.neighbors[i]].floor);
			const float hole_high = sect.neighbors[i] < 0 ? -15e15f : std::min(sect.ceil, sectors[sect.neighbors[i]].ceil);

			if (hole_high < position.y + 2.0f
				|| hole_low > position.y - get_eye_height() + 4.0f)
			{
				//Bumps into a wall! Slide along the wall.
				//This formula is from Wikipedia article "vector projection".
				const glm::vec2 vert{ vert1.x - vert2.x, vert1.y - vert2.y };

				const float vert_ls = glm::dot(vert, vert);

				const glm::vec2 new_vel{ vert * (glm::dot(vert, glm::vec2{velocity.x, velocity.z}) / vert_ls) };

				velocity.x = new_vel.x;
				velocity.z = new_vel.y;
			}
		}

		if (sect.neighbors[i] >= 0
			&& side > 0)
		{
			sector = sect.neighbors[i];
			if (sectors[sector].floor < position.y)
			{
				falling = true;
			}

			break;
		}
		else if (side > -2.5f)
		{
			if (sect.neighbors[i] < 0)
			{
				//calculate normal vector for sector line
				//we abs it to prevent the player from being sucked into the wall
				const auto d = glm::normalize(glm::vec2{ vert2.x - vert1.x, vert2.y - vert1.y }) * abs(side) * 2.5f;

				//push away from wall
				//we COULD cut off at where the velocity + p
				velocity.x += d.y * static_cast<float>(deltatime);
				velocity.z += -d.x * static_cast<float>(deltatime);
			}
			else if (std::max(sect.floor, sectors[sect.neighbors[i]].floor) > position.y - get_eye_height() + 4.0f
				|| std::min(sect.ceil, sectors[sect.neighbors[i]].ceil) < position.y + 2.0f)
			{
				//calculate normal vector for sector line
				const auto d = glm::normalize(glm::vec2{ vert2.x - vert1.x, vert2.y - vert1.y }) * abs(side) * 2.5f;

				//push away from wall
				//we COULD cut off at where the velocity + p
				velocity.x += d.y * static_cast<float>(deltatime);
				velocity.z += -d.x * static_cast<float>(deltatime);
			}
		}
	}

	position.x += velocity.x;
	position.z += velocity.z;
}

void Player::mouse_move(float xoffset, float yoffset)
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

Player::MoveDir operator&(Player::MoveDir a, Player::MoveDir b)
{
	return static_cast<Player::MoveDir>(static_cast<int>(a) & static_cast<int>(b));
}

Player::MoveDir operator|(Player::MoveDir a, Player::MoveDir b)
{
	return static_cast<Player::MoveDir>(static_cast<int>(a) | static_cast<int>(b));
}