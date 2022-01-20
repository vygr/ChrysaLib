#include "path.h"
#include <cstdlib>

extern fixed32_t abs(const fixed32_t &n);

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
		if (abs(p1.m_x + p3.m_x - p2.m_x - p2.m_x)
			+ abs(p1.m_y + p3.m_y - p2.m_y - p2.m_y) <= tol)
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
		if (abs(p1.m_x + p3.m_x - p2.m_x - p2.m_x)
			+ abs(p1.m_y + p3.m_y - p2.m_y - p2.m_y)
			+ abs(p2.m_x + p4.m_x - p3.m_x - p3.m_x)
			+ abs(p2.m_y + p4.m_y - p3.m_y - p3.m_y) <= tol)
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
		auto p1 = m_points[0];
		m_points.erase(std::remove_if(begin(m_points) + 1, end(m_points), [&] (auto &p2)
		{
			if (distance_squared_v2(p1, p2) < tol) return true;
			p1 = p2;
			return false;
		}), end(m_points));
	}
	return this;
}

Path *Path::filter_polygon(fixed32_t tol)
{
	filter_polyline(tol);
	auto len = m_points.size();
	if (len > 1 && distance_squared_v2(m_points[0], m_points[len - 1]) < tol)
	{
		m_points.pop_back();
	}
	return this;
}

std::shared_ptr<Path> Path::stroke_polyline(fixed32_t radius, fixed32_t tol, uint32_t join_style, uint32_t cap1_style, uint32_t cap2_style)
{
	filter_polyline(0.5);
	return std::make_shared<Path>(stroke_path(m_points, radius, 64, join_style, cap1_style, cap2_style));
}

std::pair<std::shared_ptr<Path>, std::shared_ptr<Path>> Path::stroke_polygon(fixed32_t radius, fixed32_t tol, uint32_t join_style)
{
	filter_polygon(0.5);
	auto points = stroke_path(m_points, radius, 64, join_style, cap_butt, cap_butt);
	auto mid = begin(points) + points.size() / 2;
	return std::make_pair(
		std::make_shared<Path>(begin(points), mid),
		std::make_shared<Path>(mid, begin(points)));
}
