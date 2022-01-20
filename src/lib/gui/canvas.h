#ifndef CANVAS_H
#define CANVAS_H

#include "../math/fixed.h"
#include "view.h"

//load flags
enum
{
	load_flag_shared = 1<< 0,
	load_flag_film = 1<< 1,
	load_flag_noswap = 1<< 2,
};

//flags
enum
{
	canvas_flag_antialias = 1 << 0,
};

//winding
enum
{
	winding_odd_even,
	winding_none_zero,
};

struct Edge
{
	Edge(fixed32_t x, int32_t ys, int32_t ye, int32_t w, fixed32_t dda)
		: m_x(x)
		, m_ys(ys)
		, m_ye(ye)
		, m_w(w)
		, m_dda(dda)
	{}
	Edge *m_next = 0;
	fixed32_t m_x;
	int32_t m_ys;
	int32_t m_ye;
	int32_t m_w;
	fixed32_t m_dda;
};

struct edge_bounds
{
	int32_t m_min_x = INT32_MAX;
	int32_t m_min_y = INT32_MAX;
	int32_t m_max_x = INT32_MIN;
	int32_t m_max_y = INT32_MIN;
};

class Pixmap;
class Texture;
class Path;

class Canvas : public View
{
public:
	Canvas(int32_t w, int32_t h, int32_t scale);
	Canvas(std::shared_ptr<Pixmap> pixmap);
	view_size pref_size() override;
	Canvas *draw(const Ctx &ctx) override;
	Canvas *swap();
	Canvas *set_clip(int32_t x, int32_t y, int32_t x1, int32_t y1);
	Canvas *set_col(uint32_t col) { m_col = col; return this; };
	Canvas *set_canvas_flags(uint32_t flags) { m_canvas_flags = flags; return this; };
	Canvas *plot(int32_t x, int32_t y);
	uint32_t pick(int32_t x, int32_t y);
	Canvas *fill();
	Canvas *fbox(int32_t x, int32_t y, int32_t w, int32_t h);
	Canvas *span_noclip(int32_t coverage, int32_t x, int32_t y, int32_t x1);
	Canvas *span(int32_t coverage, int32_t x, int32_t y, int32_t x1);
	edge_bounds set_edges(const std::vector<Path> &polygons, Vec2f p, int32_t scale);
	Canvas *fpoly(const std::vector<Path> &polygons, Vec2f p, int winding);
	std::shared_ptr<Pixmap> m_pixmap;
	std::shared_ptr<Texture> m_texture;
	std::vector<Edge> m_edges;
	std::vector<Edge*> m_edges_start;
	std::vector<uint8_t> m_coverage;
	uint32_t m_col = 0;
	uint32_t m_canvas_flags = 0;
	int32_t m_scale = 0;
	int32_t m_cx = 0;
	int32_t m_cy = 0;
	int32_t m_cx1 = 0;
	int32_t m_cy1 = 0;
};

#endif
