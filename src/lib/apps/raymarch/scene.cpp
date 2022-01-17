#include "app.h"
#include "../../gui/colors.h"
#include "../../math/vector.h"

//settings
const double eps = 0.1;
const double min_distance = 0.01;
const double clipfar = 8.0;
const double march_factor = 1.0;
const double shadow_softness = 64.0;
const double attenuation = 0.05;
const double ambient = 0.05;
const double ref_coef = 0.3;
const int32_t ref_depth = 2;
const Vec3d light_pos = {-0.1, -0.1, -3.0};

const auto ex = Vec3d{-eps, 0.0, 0.0};
const auto ey = Vec3d{0.0, -eps, 0.0};
const auto ez = Vec3d{0.0, 0.0, -eps};

//the scene
double scene(const Vec3d &p)
{
	return length_v3(sub_v3(frac_v3(p), Vec3d{0.5, 0.5, 0.5})) - 0.35;
}

double ray_march(const Vec3d &ray_origin, const Vec3d &ray_dir,
	double l, double max_l, double min_distance, double march_factor)
{
	auto i = -1;
	auto d = 1.0;
	while (++i < 1000 && d > min_distance && l < max_l)
	{
		d = scene(add_v3(ray_origin, scale_v3(ray_dir, l)));
		l += d * march_factor;
	}
	return d > min_distance ? max_l : l;
}

Vec3d get_normal(const Vec3d &p)
{
	const auto d = scene(p);
	return norm_v3(Vec3d(
		d - scene(add_v3(p, ex)),
		d - scene(add_v3(p, ey)),
		d - scene(add_v3(p, ez))));
}

double shadow(const Vec3d &ray_origin, const Vec3d &ray_dir,
	double l, double max_l, double k)
{
	auto s = 1.0;
	auto i = 1000;
	while (--i > 0)
	{
		auto h = scene(add_v3(ray_origin, scale_v3(ray_dir, l)));
		s = std::min(s, (k * h) / l);
		if (s <= 0.1 || l >= max_l) break;
		else l += h;
	}
	return std::max(s, 0.1);
}

Vec3d lighting(const Vec3d &surface_pos, const Vec3d &surface_norm, const Vec3d &cam_pos)
{
	auto obj_color = floor_v3(mod_v3(surface_pos, Vec3d{2.0, 2.0, 2.0}));
	auto light_vec = sub_v3(light_pos, surface_pos);
	auto light_dis = length_v3(light_vec);
	auto light_norm = scale_v3(light_vec, 1.0 / light_dis);
	auto light_atten = std::min(1.0 / (light_dis * light_dis * attenuation), 1.0);
	auto ref = reflect_v3(scale_v3(light_norm, -1.0), surface_norm);
	auto ss = shadow(surface_pos, light_norm, min_distance, light_dis, shadow_softness);
	auto light_col = scale_v3(Vec3d{1.0, 1.0, 1.0}, light_atten * ss);
	auto diffuse = std::max(0.0, dot_v3(surface_norm, light_norm));
	auto specular = std::max(0.0, dot_v3(ref, norm_v3(sub_v3(cam_pos, surface_pos))));
	specular = specular * specular * specular * specular;
	obj_color = scale_v3(obj_color, (diffuse * (1.0 - ambient)) + ambient);
	obj_color = add_v3(obj_color, Vec3d{specular, specular, specular});
	return mul_v3(obj_color, light_col);
}

Vec3d scene_ray(Vec3d ray_origin, Vec3d ray_dir)
{
	auto l = ray_march(ray_origin, ray_dir, 0.0, clipfar, min_distance, march_factor);
	if (l >= clipfar) return Vec3d{0.0, 0.0, 0.0};
	//diffuse lighting
	auto surface_pos = add_v3(ray_origin, scale_v3(ray_dir, l));
	auto surface_norm = get_normal(surface_pos);
	auto color = lighting(surface_pos, surface_norm, ray_origin);
	auto i = ref_depth;
	auto r = ref_coef;
	//reflections
	for (;;)
	{
		if (--i < 0) break;
		ray_origin = surface_pos;
		ray_dir = reflect_v3(ray_dir, surface_norm);
		l = ray_march(ray_origin, ray_dir, min_distance * 10.0, clipfar, min_distance, march_factor);
		if (l >= clipfar) break;
		surface_pos = add_v3(ray_origin, scale_v3(ray_dir, l));
		surface_norm = get_normal(surface_pos);
		color = add_v3(scale_v3(color, 1.0 - r), scale_v3(lighting(surface_pos, surface_norm, ray_origin), r));
		r *= ref_coef;
	}
	return clamp_v3(color, Vec3d{0.0, 0.0, 0.0}, Vec3d{1.0, 1.0, 1.0});
}

void Raymarch_App::render(Raymarch_Job_reply* body,
		uint32_t x, uint32_t y,
		uint32_t x1, uint32_t y1,
		uint32_t cw, uint32_t ch) const
{
	const auto stride = (x1 - x);
	const auto w2 = cw / 2.0;
	const auto h2 = ch / 2.0;
	const auto ray_origin = Vec3d{0.0, 0.0, -3.0};
	for (auto ry = y; ry < y1; ++ry)
	{
		for (auto rx = x; rx < x1; ++rx)
		{
			auto dx = (rx - w2) / w2;
			auto dy = (ry - h2) / h2;
			auto ray_dir = norm_v3(sub_v3(Vec3d(dx, dy, 0.0), ray_origin));
			auto col_v3 = scene_ray(ray_origin, ray_dir);
			auto col = argb_black +
				((int)(col_v3.m_x * 0xff) << 16) +
				((int)(col_v3.m_y * 0xff) << 8) +
				(int)(col_v3.m_z * 0xff);
			body->m_data[(ry - y) * stride + (rx - x)] = col;
		}
	}
}
