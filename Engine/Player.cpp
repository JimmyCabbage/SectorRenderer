#include "Player.hpp"

#include <cmath>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

#include "linemath.hpp"

glm::mat4 Player::get_view_matrix() const
{
	return glm::lookAt(position, position + front, world_up);
}

void Player::move(MoveDir dir, const float deltatime)
{
	const float speed = 50.0f * deltatime;

	glm::vec2 move_dir{ 0.0f, 0.0f };

	const glm::vec2 front_dir = glm::normalize(glm::vec2
	{
		cos(glm::radians(yaw)),
		sin(glm::radians(yaw))
	});

	const glm::vec2 right_dir = [this, &front_dir]()
	{
		const glm::vec3 temp_right = glm::normalize(glm::cross(glm::vec3{ front_dir.x, 0.0f, front_dir.y }, world_up));

		return glm::vec2{ temp_right.x, temp_right.z };
	}();

	if ((dir & MoveDir::FORWARD) == MoveDir::FORWARD)
	{
		move_dir += front_dir * speed;
	}
	if ((dir & MoveDir::BACKWARD) == MoveDir::BACKWARD)
	{
		move_dir -= front_dir * speed;
	}
	if ((dir & MoveDir::LEFT) == MoveDir::LEFT)
	{
		move_dir -= right_dir * speed;
	}
	if ((dir & MoveDir::RIGHT) == MoveDir::RIGHT)
	{
		move_dir += right_dir * speed;
	}

	velocity.x = velocity.x * (1 - 0.2f) + move_dir.x * 0.2f;
	velocity.z = velocity.z * (1 - 0.2f) + move_dir.y * 0.2f;
}

#include <iostream>

void Player::collision(const std::vector<Sector>& sectors, const float deltatime)
{
	const auto& sect = sectors[sector];

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

		//horizontal check
		if (PointSide(position.x + velocity.x, position.z + velocity.z, vert1.x, vert1.y, vert2.x, vert2.y) > 0)
		{
			const float hole_low = sect.neighbors[i] < 0 ?
				15e9f :
				std::max(sect.floor, sectors[sect.neighbors[i]].floor);
			const float hole_high = sect.neighbors[i] < 0 ?
				15e9f :
				std::min(sect.ceil, sectors[sect.neighbors[i]].ceil);

			if ((hole_high < position.y + 0.5f
				|| hole_low > position.y - EYE_HEIGHT + 1.0f))
			{
				//Bumps into a wall! Slide along the wall.
				//This formula is from Wikipedia article "vector projection".
				const float xd = vert1.x - vert2.x, yd = vert1.y - vert2.y;
				velocity.x = xd * (velocity.x * xd + yd * velocity.z) / (xd * xd + yd * yd);
				velocity.z = yd * (velocity.x * xd + yd * velocity.z) / (xd * xd + yd * yd);
			}
		}

		if (sect.neighbors[i] >= 0
			&& PointSide(position.x + velocity.x, position.z + velocity.z, vert1.x, vert1.y, vert2.x, vert2.y) > 0)
		{
				sector = sect.neighbors[i];
				break;
		}
	}

	position += velocity;

	position.y = sectors[sector].floor + EYE_HEIGHT;

	velocity = glm::vec3{ 0.0f, 0.0f, 0.0f };

	update_vectors();
}