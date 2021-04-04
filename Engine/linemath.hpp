#ifndef LINE_MATH_HPP
#define LINE_MATH_HPP

#include <glm/glm.hpp>

namespace lm
{
	struct Line2d
	{
		glm::vec2 p1, p2;
	};

	bool does_line_intersect(const Line2d& line1, const Line2d& line2);

	glm::vec2 line_intersection_point(const Line2d& line1, const Line2d& line2);
}

#endif