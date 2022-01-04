#include "path.h"

Path *Path::push_back(int32_t x, int32_t y)
{
	m_points.push_back(x);
	m_points.push_back(y);
	return this;
}

Path *Path::gen_cubic(int32_t p1x, int32_t p1y, int32_t p2x, int32_t p2y,
	int32_t p3x, int32_t p3y, int32_t p4x, int32_t p4y, int32_t tol)
{
	return this;
}
