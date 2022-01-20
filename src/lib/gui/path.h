#ifndef PATH_H
#define PATH_H

#include "../math/vector.h"
#include <vector>
#include <memory>
#include <cstddef>

class Path
{
public:
	Path() {}
	Path(std::vector<Vec2f> &&points)
		: m_points(points)
	{}
	Path(std::vector<Vec2f>::iterator i1, std::vector<Vec2f>::iterator i2)
		: m_points(i1, i2)
	{}
	Path *push_back(const Vec2f &p);
	Path *pop_back();
	Vec2f &operator[](const uint64_t &i) { return m_points[i]; }
	const Vec2f &operator[](const uint64_t &i) const { return m_points[i]; }
	size_t size() const { return m_points.size(); };
	Path *gen_quadratic(Vec2f p1, Vec2f p2, Vec2f p3, fixed32_t tol);
	Path *gen_cubic(Vec2f p1, Vec2f p2, Vec2f p3, Vec2f p4, fixed32_t tol);
	Path *filter_polyline(fixed32_t tol);
	Path *filter_polygon(fixed32_t tol);
	Path stroke_polyline(fixed32_t radius, fixed32_t tol, uint32_t join_style, uint32_t cap1_style, uint32_t cap2_style);
	std::vector<Path> stroke_polygon(fixed32_t radius, fixed32_t tol, uint32_t join_style);
	std::vector<Vec2f> m_points;
};

#endif
