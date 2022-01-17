#ifndef LIB_VECTOR_H
#define LIB_VECTOR_H

#include <cmath>
#include <vector>
#include <tuple>
#ifdef _WIN32
	#include <corecrt_math_defines.h>
#endif

//////////////
//vector types
//////////////

struct Vec2d
{
	Vec2d() : m_x(0.0), m_y(0.0) {}
	Vec2d(double x, double y) : m_x(x), m_y(y) {}
	bool operator==(const Vec2d &p) const {
		return std::tie(m_x, m_y) == std::tie(p.m_x, p.m_y); }
	bool operator!=(const Vec2d &p) const {
		return std::tie(m_x, m_y) != std::tie(p.m_x, p.m_y); }
	bool operator<(const Vec2d &p) const {
		return std::tie(m_x, m_y) < std::tie(p.m_x, p.m_y); }
	double m_x;
	double m_y;
};

struct Vec3d
{
	Vec3d() : m_x(0.0), m_y(0.0), m_z(0.0) {}
	Vec3d(double x, double y, double z) : m_x(x), m_y(y), m_z(z) {}
	bool operator==(const Vec3d &p) const {
		return std::tie(m_x, m_y, m_z) == std::tie(p.m_x, p.m_y, p.m_z); }
	bool operator!=(const Vec3d &p) const {
		return std::tie(m_x, m_y, m_z) != std::tie(p.m_x, p.m_y, p.m_z); }
	bool operator<(const Vec3d &p) const {
		return std::tie(m_x, m_y, m_z) < std::tie(p.m_x, p.m_y, p.m_z); }
	double m_x;
	double m_y;
	double m_z;
};

///////////////////////
//distance metric stuff
///////////////////////

template <class T>
auto manhattan_distance_v2(const T &p1, const T &p2)
{
	auto dx = p1.m_x - p2.m_x;
	auto dy = p1.m_y - p2.m_y;
	return double(fabs(dx) + fabs(dy));
}

template <class T>
auto manhattan_distance_v3(const T &p1, const T &p2)
{
	auto dx = p1.m_x - p2.m_x;
	auto dy = p1.m_y - p2.m_y;
	auto dz = p1.m_z - p2.m_z;
	return double(fabs(dx) + fabs(dy) + fabs(dz));
}

template <class T>
auto euclidean_distance_v2(const T &p1, const T &p2)
{
	auto dx = p1.m_x - p2.m_x;
	auto dy = p1.m_y - p2.m_y;
	return double(sqrt(dx * dx + dy * dy));
}

template <class T>
auto euclidean_distance_v3(const T &p1, const T &p2)
{
	auto dx = p1.m_x - p2.m_x;
	auto dy = p1.m_y - p2.m_y;
	auto dz = p1.m_z - p2.m_z;
	return double(sqrt(dx * dx + dy * dy + dz * dz));
}

template <class T>
auto squared_euclidean_distance_v2(const T &p1, const T &p2)
{
	auto dx = p1.m_x - p2.m_x;
	auto dy = p1.m_y - p2.m_y;
	return double(dx * dx + dy * dy);
}

template <class T>
auto squared_euclidean_distance_v3(const T &p1, const T &p2)
{
	auto dx = p1.m_x - p2.m_x;
	auto dy = p1.m_y - p2.m_y;
	auto dz = p1.m_z - p2.m_z;
	return double(dx * dx + dy * dy + dz * dz);
}

template <class T>
auto chebyshev_distance_v2(const T &p1, const T &p2)
{
	auto dx = fabs(p1.m_x - p2.m_x);
	auto dy = fabs(p1.m_y - p2.m_y);
	return std::max(dx, dy);
}

template <class T>
auto chebyshev_distance_v3(const T &p1, const T &p2)
{
	auto dx = fabs(p1.m_x - p2.m_x);
	auto dy = fabs(p1.m_y - p2.m_y);
	auto dz = fabs(p1.m_z - p2.m_z);
	auto d = std::max(dx, dy);
	return std::max(d, dz);
}

template <class T>
auto reciprocal_distance_v2(const T &p1, const T &p2)
{
	auto d = manhattan_distance_v2(p1, p2);
	if (d == 0.0) return 1.0;
	return 1.0 / d;
}

template <class T>
auto reciprocal_distance_v3(const T &p1, const T &p2)
{
	auto d = manhattan_distance_v3(p1, p2);
	if (d == 0.0) return 1.0;
	return 1.0 / d;
}

//////////////////
//specific lengths
//////////////////

template <class T>
auto add_v2(const T &p1, const T &p2)
{
	return T(p1.m_x + p2.m_x, p1.m_y + p2.m_y);
}

template <class T>
auto sub_v2(const T &p1, const T &p2)
{
	return T(p1.m_x - p2.m_x, p1.m_y - p2.m_y);
}

template <class T>
auto scale_v2(const T &p, double s)
{
	return T(p.m_x * s, p.m_y * s);
}

template <class T>
auto perp_v2(const T &p)
{
	return T(-p.m_y, p.m_x);
}

template <class T>
auto dot_v2(const T &p1, const T &p2)
{
	return p1.m_x * p2.m_x + p1.m_y * p2.m_y;
}

template <class T>
auto det_v2(const T &p1, const T &p2)
{
	return p1.m_x * p2.m_y - p1.m_y * p2.m_x;
}

template <class T>
auto length_v2(const T &p)
{
	return double(sqrt(dot_v2(p, p)));
}

template <class T>
auto norm_v2(const T &p)
{
	auto l = length_v2(p);
	if (l == 0.0) return T(0.0, 0.0);
	return scale_v2(p, 1.0 / l);
}

template <class T>
auto distance_v2(const T &p1, const T &p2)
{
	return length_v2(sub_v2(p2, p1));
}

template <class T>
auto distance_squared_v2(const T &p1, const T &p2)
{
	auto p = sub_v2(p2, p1);
	return dot_v2(p, p);
}

template <class T>
auto distance_to_line_v2(const T &p, const T &p1, const T &p2)
{
	auto lv = sub_v2(p2, p1);
	auto pv = sub_v2(p, p1);
	auto c1 = dot_v2(pv, lv);
	if (c1 <= 0.0) return distance_v2(p, p1);
	auto c2 = dot_v2(lv, lv);
	if (c2 <= c1) return distance_v2(p, p2);
	return distance_v2(p, add_v2(p1, scale_v2(lv, c1/c2)));
}

template <class T>
auto distance_squared_to_line_v2(const T &p, const T &p1, const T &p2)
{
	auto lv = sub_v2(p2, p1);
	auto pv = sub_v2(p, p1);
	auto c1 = dot_v2(pv, lv);
	if (c1 <= 0.0) return distance_squared_v2(p, p1);
	auto c2 = dot_v2(lv, lv);
	if (c2 <= c1) return distance_squared_v2(p, p2);
	return distance_squared_v2(p, add_v2(p1, scale_v2(lv, c1/c2)));
}

template <class T>
auto collide_lines_v2(const T &l1_p1, const T &l1_p2, const T &l2_p1, const T &l2_p2)
{
	auto av = sub_v2(l1_p2, l1_p1);
	auto bv = sub_v2(l2_p2, l2_p1);
	auto cv = sub_v2(l2_p2, l1_p1);
	auto axb = det_v2(av, bv);
	auto axc = det_v2(av, cv);
	auto cxb = det_v2(cv, bv);
	if (axb == 0.0) return false;
	if (axb > 0.0)
	{
		if ((axc < 0.0) || (axc > axb)) return false;
		if ((cxb < 0.0) || (cxb > axb)) return false;
	}
	else
	{
		if ((axc > 0.0) || (axc < axb)) return false;
		if ((cxb > 0.0) || (cxb < axb)) return false;
	}
	return true;
}

template <class T>
bool collide_thick_lines_v2(const T &tl1_p1, const T &tl1_p2,
	 						const T &tl2_p1, const T &tl2_p2, double r)
{
	if (collide_lines_v2(tl1_p1, tl1_p2, tl2_p1, tl2_p2)) return true;
	r *= r;
	if (distance_squared_to_line_v2(tl2_p1, tl1_p1, tl1_p2) <= r) return true;
	if (distance_squared_to_line_v2(tl2_p2, tl1_p1, tl1_p2) <= r) return true;
	if (distance_squared_to_line_v2(tl1_p1, tl2_p1, tl2_p2) <= r) return true;
	if (distance_squared_to_line_v2(tl1_p2, tl2_p1, tl2_p2) <= r) return true;
	return false;
}

template <class T>
auto add_v3(const T &p1, const T &p2)
{
	return T(p1.m_x + p2.m_x, p1.m_y + p2.m_y, p1.m_z + p2.m_z);
}

template <class T>
auto sub_v3(const T &p1, const T &p2)
{
	return T(p1.m_x - p2.m_x, p1.m_y - p2.m_y, p1.m_z - p2.m_z);
}

template <class T>
auto mul_v3(const T &p1, const T &p2)
{
	return T(p1.m_x * p2.m_x, p1.m_y * p2.m_y, p1.m_z * p2.m_z);
}

template <class T>
auto scale_v3(const T &p, double s)
{
	return T(p.m_x * s, p.m_y * s, p.m_z * s);
}

template <class T>
auto mod_v3(const T &p1, const T &p2)
{
	auto x = std::fmod(p1.m_x, p2.m_x);
	auto y = std::fmod(p1.m_y, p2.m_y);
	auto z = std::fmod(p1.m_z, p2.m_z);
	if (x < 0.0) x += p2.m_x;
	if (y < 0.0) y += p2.m_y;
	if (z < 0.0) z += p2.m_z;
	return T(x, y, z);
}

template <class T>
auto frac_v3(const T &p)
{
	double intpart;
	auto x = std::modf(p.m_x, &intpart);
	auto y = std::modf(p.m_y, &intpart);
	auto z = std::modf(p.m_z, &intpart);
	if (x < 0.0) x += 1.0;
	if (y < 0.0) y += 1.0;
	if (z < 0.0) z += 1.0;
	return T(x, y, z);
}

template <class T>
auto floor_v3(const T &p)
{
	return T(std::floor(p.m_x), std::floor(p.m_y), std::floor(p.m_z));
}

template <class T>
auto dot_v3(const T &p1, const T &p2)
{
	return p1.m_x * p2.m_x + p1.m_y * p2.m_y + p1.m_z * p2.m_z;
}

template <class T>
auto length_v3(const T &p)
{
	return double(sqrt(dot_v3(p, p)));
}

template <class T>
auto norm_v3(const T &p)
{
	auto l = length_v3(p);
	if (l == 0.0) return T(0.0, 0.0, 0.0);
	return scale_v3(p, 1.0 / l);
}

template <class T>
auto reflect_v3(const T &p, const T &n)
{
	return sub_v3(p, scale_v3(n, dot_v3(p, n) * 2.0));
}

template <class T>
auto clamp_v3(const T &p1, const T &p2, const T &p3)
{
	return T(
		std::min(std::max(p1.m_x, p2.m_x), p3.m_x),
		std::min(std::max(p1.m_y, p2.m_y), p3.m_y),
		std::min(std::max(p1.m_z, p2.m_z), p3.m_z));
}

////////////////////
//generic path stuff
////////////////////

template <class T>
auto circle_as_lines(const T &p, double radius, int resolution)
{
	auto out_points = std::vector<T>{}; out_points.reserve(resolution+1);
	auto rvx = 0.0;
	auto rvy = radius;
	for (auto i = 0; i <= resolution; ++i)
	{
		auto angle = (i * M_PI * 2.0) / resolution;
		auto s = double(sin(angle));
		auto c = double(cos(angle));
		auto rv = T(rvx*c - rvy*s, rvx*s + rvy*c);
		out_points.push_back(sub_v2(p, rv));
	}
	out_points.push_back(out_points[0]);
	return out_points;
}

template <class T>
auto torus_as_tristrip(const T &p, double radius1, double radius2, int resolution)
{
	auto out_points = std::vector<T>{}; out_points.reserve(resolution*2+2);
	auto rvx1 = 0.0;
	auto rvy1 = radius1;
	auto rvx2 = 0.0;
	auto rvy2 = radius2;
	for (auto i = 0; i <= resolution; ++i)
	{
		auto angle = (i * M_PI * 2.0) / resolution;
		auto s = double(sin(angle));
		auto c = double(cos(angle));
		auto rv1 = T(rvx1*c - rvy1*s, rvx1*s + rvy1*c);
		auto rv2 = T(rvx2*c - rvy2*s, rvx2*s + rvy2*c);
		out_points.push_back(sub_v2(p, rv1));
		out_points.push_back(sub_v2(p, rv2));
	}
	out_points.push_back(out_points[0]);
	out_points.push_back(out_points[1]);
	return out_points;
}

template <class T>
auto circle_as_trifan(const T &p, double radius, int resolution)
{
	auto out_points = std::vector<T>{}; out_points.reserve(resolution*2+2);
	auto rvx1 = 0.0;
	auto rvy1 = radius;
	out_points.push_back(p);
	for (auto i = 0; i <= resolution; ++i)
	{
		auto angle = (i * M_PI * 2.0) / resolution;
		auto s = double(sin(angle));
		auto c = double(cos(angle));
		auto rv1 = T(rvx1*c - rvy1*s, rvx1*s + rvy1*c);
		out_points.push_back(sub_v2(p, rv1));
	}
	out_points.push_back(out_points[0]);
	return out_points;
}

template <class T>
auto thicken_path_as_lines(const std::vector<T> &path, double radius, int capstyle, int joinstyle, int resolution)
{
	if (radius == 0.0) radius = 0.00000001;
	auto index = 0;
	auto step = 1;
	auto out_points = std::vector<T>{};
	for (;;)
	{
		auto p1 = path[index];
		index += step;
		auto p2 = path[index];
		index += step;
		auto l2_v = sub_v2(p2, p1);
		auto l2_pv = perp_v2(l2_v);
		auto l2_npv = norm_v2(l2_pv);
		auto rv = scale_v2(l2_npv, radius);
		switch (capstyle)
		{
			case 0:
			{
				//butt cap
				out_points.push_back(sub_v2(p1, rv));
				out_points.push_back(add_v2(p1, rv));
				break;
			}
			case 1:
			{
				//square cap
				auto p0 = add_v2(p1, perp_v2(rv));
				out_points.push_back(sub_v2(p0, rv));
				out_points.push_back(add_v2(p0, rv));
				break;
			}
			case 2:
			{
				//triangle cap
				out_points.push_back(sub_v2(p1, rv));
				out_points.push_back(add_v2(p1, perp_v2(rv)));
				out_points.push_back(add_v2(p1, rv));
				break;
			}
			default:
			{
				//round cap
				auto rvx = rv.m_x;
				auto rvy = rv.m_y;
				for (auto i = 0; i <= resolution; ++i)
				{
					auto angle = (i * -M_PI) / resolution;
					auto s = double(sin(angle));
					auto c = double(cos(angle));
					auto rv = T(rvx*c - rvy*s, rvx*s + rvy*c);
					out_points.push_back(sub_v2(p1, rv));
				}
			}
		}
		while ((index != -1) && (index != static_cast<int>(path.size())))
		{
			auto p1 = p2;
			auto l1_v = l2_v;
			auto l1_npv = l2_npv;
			p2 = path[index];
			index += step;
			l2_v = sub_v2(p2, p1);
			l2_pv = perp_v2(l2_v);
			l2_npv = norm_v2(l2_pv);
			auto nbv = norm_v2(scale_v2(add_v2(l1_npv, l2_npv), 0.5f));
			auto c = dot_v2(nbv, norm_v2(l1_v));
			if (c <= 0.0) goto mitre_join;
			switch (joinstyle)
			{
				case 0:
				{
				mitre_join:
					//mitre join
					auto s = double(sin(acos(c)));
					auto bv = scale_v2(nbv, radius/s);
					out_points.push_back(add_v2(p1, bv));
					break;
				}
				case 1:
				{
					//bevel join
					out_points.push_back(add_v2(p1, scale_v2(l1_npv, radius)));
					out_points.push_back(add_v2(p1, scale_v2(l2_npv, radius)));
					break;
				}
				default:
				{
					//round join
					auto rv = scale_v2(l1_npv, radius);
					auto rvx = rv.m_x;
					auto rvy = rv.m_y;
					auto theta = -double(acos(dot_v2(l1_npv, l2_npv)));
					auto segs = int((theta/-M_PI)*resolution) + 1;
					for (auto i = 0; i <= segs; ++i)
					{
						auto angle = (i * theta) / segs;
						auto s = double(sin(angle));
						auto c = double(cos(angle));
						auto rv = T(rvx*c - rvy*s, rvx*s + rvy*c);
						out_points.push_back(add_v2(p1, rv));
					}
				}
			}
		}
		if (step < 0) break;
		step = -step;
		index += step;
	}
	out_points.push_back(out_points[0]);
	return out_points;
}

template <class T>
auto thicken_path_as_tristrip(const std::vector<T> &path, double radius, int capstyle, int joinstyle, int resolution)
{
	if (radius == 0.0) radius = 0.00000001f;
	auto index = 0;
	auto step = 1;
	auto out_points = std::vector<T>{};
	for (;;)
	{
		auto p1 = path[index];
		index += step;
		auto p2 = path[index];
		index += step;
		auto l2_v = sub_v2(p2, p1);
		auto l2_pv = perp_v2(l2_v);
		auto l2_npv = norm_v2(l2_pv);
		auto rv = scale_v2(l2_npv, radius);
		switch (capstyle)
		{
			case 0:
			{
				//butt cap
				out_points.push_back(p1);
				out_points.push_back(sub_v2(p1, rv));
				out_points.push_back(p1);
				out_points.push_back(add_v2(p1, rv));
				break;
			}
			case 1:
			{
				//square cap
				auto p0 = add_v2(p1, perp_v2(rv));
				out_points.push_back(p0);
				out_points.push_back(sub_v2(p0, rv));
				out_points.push_back(p0);
				out_points.push_back(add_v2(p0, rv));
				break;
			}
			case 2:
			{
				//triangle cap
				out_points.push_back(p1);
				out_points.push_back(sub_v2(p1, rv));
				out_points.push_back(p1);
				out_points.push_back(add_v2(p1, perp_v2(rv)));
				out_points.push_back(p1);
				out_points.push_back(add_v2(p1, rv));
				break;
			}
			default:
			{
				//round cap
				auto rvx = rv.m_x;
				auto rvy = rv.m_y;
				for (auto i = 0; i <= resolution; ++i)
				{
					auto angle = (i * -M_PI) / resolution;
					auto s = double(sin(angle));
					auto c = double(cos(angle));
					auto rv = T(rvx*c - rvy*s, rvx*s + rvy*c);
					out_points.push_back(p1);
					out_points.push_back(sub_v2(p1, rv));
				}
			}
		}
		while ((index != -1) && (index != static_cast<int>(path.size())))
		{
			auto p1 = p2;
			auto l1_v = l2_v;
			auto l1_npv = l2_npv;
			p2 = path[index];
			index += step;
			l2_v = sub_v2(p2, p1);
			l2_pv = perp_v2(l2_v);
			l2_npv = norm_v2(l2_pv);
			auto nbv = norm_v2(scale_v2(add_v2(l1_npv, l2_npv), 0.5f));
			auto c = dot_v2(nbv, norm_v2(l1_v));
			if (c <= 0.0) goto mitre_join;
			switch (joinstyle)
			{
				case 0:
				{
				mitre_join:
					//mitre join
					auto s = double(sin(acos(c)));
					auto bv = scale_v2(nbv, radius/s);
					out_points.push_back(p1);
					out_points.push_back(add_v2(p1, bv));
					break;
				}
				case 1:
				{
					//bevel join
					out_points.push_back(p1);
					out_points.push_back(add_v2(p1, scale_v2(l1_npv, radius)));
					out_points.push_back(p1);
					out_points.push_back(add_v2(p1, scale_v2(l2_npv, radius)));
					break;
				}
				default:
				{
					//round join
					auto rv = scale_v2(l1_npv, radius);
					auto rvx = rv.m_x;
					auto rvy = rv.m_y;
					auto theta = -double(acos(dot_v2(l1_npv, l2_npv)));
					auto segs = int((theta/-M_PI)*resolution) + 1;
					for (auto i = 0; i <= segs; ++i)
					{
						auto angle = (i * theta) / segs;
						auto s = double(sin(angle));
						auto c = double(cos(angle));
						auto rv = T(rvx*c - rvy*s, rvx*s + rvy*c);
						out_points.push_back(p1);
						out_points.push_back(add_v2(p1, rv));
					}
				}
			}
		}
		if (step < 0) break;
		step = -step;
		index += step;
	}
	out_points.push_back(out_points[0]);
	out_points.push_back(out_points[1]);
	return out_points;
}

#endif
