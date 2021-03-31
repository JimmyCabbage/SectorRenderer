#ifndef SECTOR_OPENGL_HPP
#define SECTOR_OPENGL_HPP

#include <vector>

#include <glm/glm.hpp>

enum class MaterialType : uint16_t
{
	STONE = 0,
	WOOD,
	BRICK,
	METAL,
	CONCRETE
};

struct Sector
{
	float ceil, floor;

	MaterialType wall_type, ceil_type, floor_type;

	std::vector<glm::vec2> vertices;
	std::vector<int32_t> neighbors;
};

#endif