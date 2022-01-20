#include "vector.h"

Vec2f::Vec2f(const Vec2F &p)
	: m_x(p.m_x)
	, m_y(p.m_y)
{}

Vec2F::Vec2F(const Vec2f &p)
	: m_x(p.m_x)
	, m_y(p.m_y)
{}
