#include "vector.h"

Vec2f::Vec2f(const Vec2F &p)
	: m_x(p.m_x)
	, m_y(p.m_y)
{}

Vec2F::Vec2F(const Vec2f &p)
	: m_x(p.m_x)
	, m_y(p.m_y)
{}

template <>
auto norm_v2(const Vec2f &p)
{
	auto l = length_v2(p);
	if (l == fixed32_t(0)) return Vec2f(0, 0);
	return scale_v2(p, fixed32_t(1.0) / l);
}

template <>
auto norm_v2(const Vec2F &p)
{
	auto l = length_v2(p);
	if (l == fixed64_t(0)) return Vec2F(0, 0);
	return scale_v2(p, fixed64_t(1.0) / l);
}
