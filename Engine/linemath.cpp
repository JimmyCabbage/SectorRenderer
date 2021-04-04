#include "linemath.hpp"

#include <stdexcept>
#include <algorithm>

//two funcs taken from https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
bool on_segment(glm::vec2 p, glm::vec2 q, glm::vec2 r)
{
	if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
		q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y))
	{
		return true;
	}

	return false;
}

float orientation(glm::vec2 p, glm::vec2 q, glm::vec2 r)
{
	float val = (q.y - p.y) * (r.x - q.x) -
		(q.x - p.x) * (r.y - q.y);

	if (val == 0.0f) return 0;

	return (val > 0.0f) ? 1.0f : 2.0f;
}

namespace lm
{
	bool does_line_intersect(const Line2d& line1, const Line2d& line2)
	{
		const float a = line1.p2.y - line1.p1.y;
		const float b = line1.p1.x - line1.p2.x;

		const float a1 = line2.p2.y - line2.p1.y;
		const float b1 = line2.p1.x - line2.p2.x;

		float det = a * b1 - a1 * b;
		if (det == 0.0f) return false;
		else return true;
	}

	glm::vec2 line_intersection_point(const Line2d& line1, const Line2d& line2)
	{
		const float a = line1.p2.y - line1.p1.y;
		const float b = line1.p1.x - line1.p2.x;

		const float a1 = line2.p2.y - line2.p1.y;
		const float b1 = line2.p1.x - line2.p2.x;

		float det = a * b1 - a1 * b;
		if (det == 0.0f) throw std::logic_error("lines given to line_intersection_point are parallel/coincident");

		const float c = a * line1.p1.x + b * line1.p1.y;
		const float c1 = a1 * line2.p1.x + b1 * line2.p1.y;;

		return glm::vec2
		{
			(b1 * c - b * c1) / det,
			(a * c1 - a1 * c) / det
		};
	}
}