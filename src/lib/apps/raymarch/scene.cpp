#include "app.h"
#include "../../gui/colors.h"
#include "../../math/vector.h"

point_3d frac_3d(const point_3d &p);
point_3d floor_3d(const point_3d &p);
point_3d sub_3d(const point_3d &p1, const point_3d &p2);
point_3d add_3d(const point_3d &p1, const point_3d &p2);
point_3d mul_3d(const point_3d &p1, const point_3d &p2);
point_3d mod_3d(const point_3d &p1, const point_3d &p2);
point_3d clamp_3d(const point_3d &p1, const point_3d &p2, const point_3d &p3);
double dot_3d(const point_3d &p1, const point_3d &p2);
point_3d scale_3d(const point_3d &p, double s);
point_3d reflect_3d(const point_3d &p, const point_3d &n);
point_3d norm_3d(const point_3d &p);
double length_3d(const point_3d &p);

const double eps = 0.1;
const double min_distance = 0.01;
const double clipfar = 8.0;
const double march_factor = 1.0;
const double shadow_softness = 64.0;
const double attenuation = 0.05;
const double ambient = 0.05;
const double ref_coef = 0.25;
const int32_t ref_depth = 1;
const point_3d light_pos = {-0.1, -0.1, -3.0};

//the scene
double scene(const point_3d &p)
{
	return length_3d(sub_3d(frac_3d(p), point_3d{0.5, 0.5, 0.5})) - 0.35;
}

double ray_march(const point_3d &ray_origin, const point_3d &ray_dir,
	double l, double max_l, double min_distance, double march_factor)
{
	auto i = -1;
	auto d = 1.0;
	while (++i < 1000 && d > min_distance && l < max_l)
	{
		d = scene(add_3d(ray_origin, scale_3d(ray_dir, l)));
		l += d * march_factor;
	}
	return d > min_distance ? max_l : l;
}

point_3d get_normal(point_3d p)
{
	auto d = scene(p);
	return norm_3d(point_3d(
		d - scene(add_3d(p, point_3d{-eps, 0.0, 0.0})),
		d - scene(add_3d(p, point_3d{0.0, -eps, 0.0})),
		d - scene(add_3d(p, point_3d{0.0, 0.0, -eps}))));
}

double shadow(const point_3d &ray_origin, const point_3d &ray_dir,
	double l, double max_l, double k)
{
	auto s = 1.0;
	auto i = 1000;
	while (--i > 0)
	{
		auto h = scene(add_3d(ray_origin, scale_3d(ray_dir, l)));
		s = std::min(s, (k * h) / l);
		if (s <= 0.1 || l >= max_l) break;
		else l += h;
	}
	return std::max(s, 0.1);
}

point_3d lighting(const point_3d &surface_pos, const point_3d &surface_norm, const point_3d &cam_pos)
{
	auto obj_color = floor_3d(mod_3d(surface_pos, point_3d{2.0, 2.0, 2.0}));
	auto light_vec = sub_3d(light_pos, surface_pos);
	auto light_dis = length_3d(light_vec);
	auto light_norm = scale_3d(light_vec, 1.0 / light_dis);
	auto light_atten = std::min(1.0 / (light_dis * light_dis * attenuation), 1.0);
	auto ref = reflect_3d(scale_3d(light_norm, -1.0), surface_norm);
	auto ss = shadow(surface_pos, light_norm, min_distance, light_dis, shadow_softness);
	auto light_col = scale_3d(point_3d{1.0, 1.0, 1.0}, light_atten * ss);
	auto diffuse = std::max(0.0, dot_3d(surface_norm, light_norm));
	auto specular = std::max(0.0, dot_3d(ref, norm_3d(sub_3d(cam_pos, surface_pos))));
	specular = specular * specular * specular * specular;
	obj_color = scale_3d(obj_color, (diffuse * (1.0 - ambient)) + ambient);
	obj_color = add_3d(obj_color, point_3d{specular, specular, specular});
	return mul_3d(obj_color, light_col);
}

point_3d scene_ray(point_3d ray_origin, point_3d ray_dir)
{
	auto l = ray_march(ray_origin, ray_dir, 0.0, clipfar, min_distance, march_factor);
	if (l >= clipfar) return point_3d{1.0, 0.0, 0.0};
	//diffuse lighting
	auto surface_pos = add_3d(ray_origin, scale_3d(ray_dir, l));
	auto surface_norm = get_normal(surface_pos);
	auto color = lighting(surface_pos, surface_norm, ray_origin);
	auto i = ref_depth;
	auto r = ref_coef;
	//reflections
	for (;;)
	{
		if (--i < 0) break;
		ray_origin = surface_pos;
		ray_dir = reflect_3d(ray_dir, surface_norm);
		auto l = ray_march(ray_origin, ray_dir, min_distance * 10.0, clipfar, min_distance, march_factor);
		if (l >= clipfar) break;
		surface_pos = add_3d(ray_origin, scale_3d(ray_dir, l));
		surface_norm = get_normal(surface_pos);
		color = add_3d(scale_3d(color, 1.0 - r), scale_3d(lighting(surface_pos, surface_norm, ray_origin), r));
		r += ref_coef;
	}
	return clamp_3d(color, point_3d{0.0, 0.0, 0.0}, point_3d{1.0, 1.0, 1.0});
}

void Raymarch_App::render(Raymarch_Job_reply* body,
		uint32_t x, uint32_t y,
		uint32_t x1, uint32_t y1,
		uint32_t cw, uint32_t ch) const
{
	auto stride = (x1 - x);
	auto w2 = cw / 2.0;
	auto h2 = ch / 2.0;
	for (auto ry = y; ry < y1; ++ry)
	{
		for (auto rx = x; rx < x1; ++rx)
		{
			auto ray_origin = point_3d{0.0, 0.0, -3.0};
			auto ray_dir = norm_3d(sub_3d(point_3d((rx - w2) / w2, (ry - h2) / h2, 0.0), ray_origin));
			auto col_3d = scene_ray(ray_origin, ray_dir);
			auto col = argb_black +
				((int)(col_3d.m_x * 0xff) << 16) +
				((int)(col_3d.m_y * 0xff) << 8) +
				(int)(col_3d.m_z * 0xff);
			body->m_data[(ry - y) * stride + (rx - x)] = col;
		}
	}
}
