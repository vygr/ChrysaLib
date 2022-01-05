#ifndef PATH_H
#define PATH_H

#include "../settings.h"
#include <vector>

class Path
{
public:
	Path() {}
	Path *push_back(fixed32_t x, fixed32_t y);
	fixed32_t &operator[](const fixed32_t &i) { return m_points[i]; }
	const fixed32_t &operator[](const fixed32_t &i) const { return m_points[i]; }
	size_t size() const { return m_points.size(); };
	Path *gen_quadratic(fixed32_t p1x, fixed32_t p1y, fixed32_t p2x, fixed32_t p2y,
		fixed32_t p3x, fixed32_t p3y, fixed32_t tol);
	Path *gen_cubic(fixed32_t p1x, fixed32_t p1y, fixed32_t p2x, fixed32_t p2y,
		fixed32_t p3x, fixed32_t p3y, fixed32_t p4x, fixed32_t p4y, fixed32_t tol);
	std::vector<fixed32_t> m_points;
};

#endif
