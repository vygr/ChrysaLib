#ifndef PATH_H
#define PATH_H

#include <vector>

class Path
{
public:
	Path() {}
	Path *push_back(int32_t x, int32_t y);
	int32_t &operator[](const int32_t &i) { return m_points[i]; }
	const int32_t &operator[](const int32_t &i) const { return m_points[i]; }
	size_t size() const { return m_points.size(); };
	Path *gen_cubic(int32_t p1x, int32_t p1y, int32_t p2x, int32_t p2y,
		int32_t p3x, int32_t p3y, int32_t p4x, int32_t p4y, int32_t tol);
	std::vector<int32_t> m_points;
};

#endif
