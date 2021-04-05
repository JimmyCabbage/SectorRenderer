#include "Player.hpp"

#include <cmath>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Player::get_view_matrix() const
{
	return glm::lookAt(position, position + front, world_up);
}

void Player::move(MoveDir dir, const double deltatime)
{
	const float speed = 10.0f * deltatime;

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

void Player::jump(const double deltatime)
{
	if (!falling)
	{
		velocity.y += 10.0f * deltatime;
		falling = true;
	}
}

void Player::collision(const std::vector<Sector>& sectors, const double deltatime)
{
	const auto& sect = sectors[sector];

	//horizontol check
	if (falling) velocity.y -= 0.005f * deltatime;

	const float nextz = position.y + velocity.y;
	if (velocity.y < 0 && nextz < sectors[sector].floor + EYE_HEIGHT)
	{
		position.y = sectors[sector].floor + EYE_HEIGHT;
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

		if (side > -2.0f)
		{
			const float hole_low = sect.neighbors[i] < 0 ? 15e15f : std::max(sect.floor, sectors[sect.neighbors[i]].floor);
			const float hole_high = sect.neighbors[i] < 0 ? -15e15f : std::min(sect.ceil, sectors[sect.neighbors[i]].ceil);

			if ((hole_high < position.y + 1.0f
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
			&& side > 0)
		{
			sector = sect.neighbors[i];
			if (sectors[sector].floor < position.y)
			{
				falling = true;
			}

			break;
		}
		else if (side > -4.0f
			&& sect.neighbors[i] < 0)
		{
			//calculate normal vector for sector line
			const auto d = glm::normalize(glm::vec2{ vert2.x - vert1.x, vert2.y - vert1.y }) * 10.0f;

			//push away from wall
			//we COULD cut of at where the velocity + p
			velocity.x += d.y * deltatime;
			velocity.z += -d.x  * deltatime;
		}
	}

	position.x += velocity.x;
	position.z += velocity.z;
}