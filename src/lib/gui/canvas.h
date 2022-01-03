#ifndef CANVAS_H
#define CANVAS_H

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
	int32_t m_x;
	int32_t m_ys;
	int32_t m_ye;
	int32_t m_w;
	int32_t m_dda;
};

class Pixmap;
class Texture;

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
	Canvas *set_canvas_flags(uint32_t flags) { m_flags = flags; return this; };
	Canvas *plot(int32_t x, int32_t y);
	uint32_t pick(int32_t x, int32_t y);
	Canvas *span_noclip(int32_t coverage, int32_t x, int32_t y, int32_t x1);
	Canvas *span(int32_t coverage, int32_t x, int32_t y, int32_t x1);

	std::shared_ptr<Pixmap> m_pixmap;
	std::shared_ptr<Texture> m_texture;
	std::vector<std::forward_list<Edge>> m_edges;
	uint32_t m_col = 0;
	uint32_t m_flags = 0;
	int32_t m_scale = 0;
	int32_t m_cx = 0;
	int32_t m_cy = 0;
	int32_t m_cx1 = 0;
	int32_t m_cy1 = 0;
};

#endif
