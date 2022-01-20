#include "path.h"
#include <cstdlib>

Path *Path::push_back(const Vec2f &p)
{
	m_points.push_back(p);
	return this;
}

Path *Path::pop_back()
{
	m_points.pop_back();
	return this;
}

Path *Path::gen_quadratic(Vec2f p1, Vec2f p2, Vec2f p3, fixed32_t tol)
{
	auto stack = std::vector<Vec2f>{};

	//output first point
	push_back(p1);

	for (;;)
	{
		//calculate the mid-path
		auto p12 = asr_v2(add_v2(p1, p2), 1);
		auto p23 = asr_v2(add_v2(p2, p3), 1);
		auto p123 = asr_v2(add_v2(p12, p23), 1);

		//flatness test
		if (fixed32_t::abs(p1.m_x + p3.m_x - p2.m_x - p2.m_x)
			+ fixed32_t::abs(p1.m_y + p3.m_y - p2.m_y - p2.m_y) <= tol)
		{
			//output point
			push_back(p123);
		}
		else
		{
			//continue subdivision
			stack.push_back(p123);
			stack.push_back(p23);
			stack.push_back(p3);
			stack.push_back(p1);
			stack.push_back(p12);
			stack.push_back(p123);
		}

		//finished ?
		if (stack.empty()) break;

		p3 = stack.back(), stack.pop_back();
		p2 = stack.back(), stack.pop_back();
		p1 = stack.back(), stack.pop_back();
	}

	//output last point
	push_back(p3);
	return this;
}

Path *Path::gen_cubic(Vec2f p1, Vec2f p2, Vec2f p3, Vec2f p4, fixed32_t tol)
{
	auto stack = std::vector<Vec2f>{};

	//output first point
	push_back(p1);

	for (;;)
	{
		//calculate the mid-path
		auto p12 = asr_v2(add_v2(p1, p2), 1);
		auto p23 = asr_v2(add_v2(p2, p3), 1);
		auto p34 = asr_v2(add_v2(p3, p4), 1);
		auto p123 = asr_v2(add_v2(p12, p23), 1);
		auto p234 = asr_v2(add_v2(p23, p34), 1);
		auto p1234 = asr_v2(add_v2(p123, p234), 1);

		//flatness test
		if (fixed32_t::abs(p1.m_x + p3.m_x - p2.m_x - p2.m_x)
			+ fixed32_t::abs(p1.m_y + p3.m_y - p2.m_y - p2.m_y)
			+ fixed32_t::abs(p2.m_x + p4.m_x - p3.m_x - p3.m_x)
			+ fixed32_t::abs(p2.m_y + p4.m_y - p3.m_y - p3.m_y) <= tol)
		{
			//output point
			push_back(p1234);
		}
		else
		{
			//continue subdivision
			stack.push_back(p1234);
			stack.push_back(p234);
			stack.push_back(p34);
			stack.push_back(p4);
			stack.push_back(p1);
			stack.push_back(p12);
			stack.push_back(p123);
			stack.push_back(p1234);
		}

		//finished ?
		if (stack.empty()) break;

		p4 = stack.back(), stack.pop_back();
		p3 = stack.back(), stack.pop_back();
		p2 = stack.back(), stack.pop_back();
		p1 = stack.back(), stack.pop_back();
	}

	//output last point
	push_back(p4);
	return this;
}

Path *Path::filter_polyline(fixed32_t tol)
{
	if (m_points.size() > 1)
	{
		tol *= tol;
		auto x1 = m_points[0];
		auto y1 = m_points[1];
	}
	return this;
}

Path *Path::filter_polygon(fixed32_t tol)
{
	return this;
}
