#ifndef SECTOR_OPENGL_HPP
#define SECTOR_OPENGL_HPP

#include <vector>

#include <glm/glm.hpp>

struct Sector
{
	float ceil, floor;

	uint32_t wall_type, ceil_type, floor_type;

	std::vector<glm::vec2> vertices;
	std::vector<int32_t> neighbors;
};

#endif