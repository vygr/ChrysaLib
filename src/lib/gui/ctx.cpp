#include "ctx.h"
#include "view.h"
#include "../../host/sdl_dummy.h"

extern void host_gui_box(const SDL_Rect *rect);
extern void host_gui_filled_box(const SDL_Rect *rect);
extern void host_gui_set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
extern void host_gui_set_texture_color(void *t, uint8_t r, uint8_t g, uint8_t b);
extern void host_gui_blit(void *t, const SDL_Rect *srect, const SDL_Rect *drect);
extern void host_gui_set_clip(const SDL_Rect *rect);

uint32_t Ctx::darker(uint32_t col) const
{
	auto alpha = col & 0xff000000;
	return ((col & 0xfefefe) >> 1) + alpha;
}

uint32_t Ctx::brighter(uint32_t col) const
{
	auto alpha = col & 0xff000000;
	return ((col & 0xfefefe) >> 1) + alpha + 0x808080;
}

const Ctx &Ctx::set_color(uint32_t col) const
{
	uint8_t a = (col >> 24) & 0xff;
	uint8_t r = (col >> 16) & 0xff;
	uint8_t g = (col >> 8) & 0xff;
	uint8_t b = col & 0xff;
	host_gui_set_color(r, g, b, a);
	return *this;
}

const Ctx &Ctx::box(int32_t x, int32_t y, int32_t w, int32_t h) const
{
	SDL_Rect drect;
	SDL_Rect clip_rect;
	x += m_x;
	y += m_y;
	drect.x = x;
	drect.y = y;
	drect.w = w;
	drect.h = h;
	for (auto &rect : m_region->m_region)
	{
		//continue if out of bounds
		if (rect.m_x1 <= drect.x
			|| rect.m_y1 <= drect.y
			|| rect.m_x >= drect.x + drect.w
			|| rect.m_y >= drect.y + drect.h) continue;
		//set clip to this region
		clip_rect.x = rect.m_x;
		clip_rect.y = rect.m_y;
		clip_rect.w = rect.m_x1 - rect.m_x;
		clip_rect.h = rect.m_y1 - rect.m_y;
		host_gui_set_clip(&clip_rect);
		//and draw
		host_gui_box(&drect);
	}
	return *this;
}

const Ctx &Ctx::filled_box(int32_t x, int32_t y, int32_t w, int32_t h) const
{
	SDL_Rect drect;
	SDL_Rect clip_rect;
	x += m_x;
	y += m_y;
	drect.x = x;
	drect.y = y;
	drect.w = w;
	drect.h = h;
	for (auto &rect : m_region->m_region)
	{
		//continue if out of bounds
		if (rect.m_x1 <= drect.x
			|| rect.m_y1 <= drect.y
			|| rect.m_x >= drect.x + drect.w
			|| rect.m_y >= drect.y + drect.h) continue;
		//set clip to this region
		clip_rect.x = rect.m_x;
		clip_rect.y = rect.m_y;
		clip_rect.w = rect.m_x1 - rect.m_x;
		clip_rect.h = rect.m_y1 - rect.m_y;
		host_gui_set_clip(&clip_rect);
		//and draw
		host_gui_filled_box(&drect);
	}
	return *this;
}

const Ctx &Ctx::blit(void *texture, uint32_t col, int32_t x, int32_t y, int32_t w, int32_t h) const
{
	SDL_Rect drect;
	SDL_Rect srect;
	SDL_Rect clip_rect;
	x += m_x;
	y += m_y;
	drect.x = x;
	drect.y = y;
	drect.w = w;
	drect.h = h;
	srect.x = 0;
	srect.y = 0;
	srect.w = w;
	srect.h = h;

	uint8_t r = (col >> 16) & 0xff;
	uint8_t g = (col >> 8) & 0xff;
	uint8_t b = col & 0xff;
	host_gui_set_texture_color(texture, r, g, b);

	for (auto &rect : m_region->m_region)
	{
		//continue if out of bounds
		if (rect.m_x1 <= drect.x
			|| rect.m_y1 <= drect.y
			|| rect.m_x >= drect.x + drect.w
			|| rect.m_y >= drect.y + drect.h) continue;
		//set clip to this region
		clip_rect.x = rect.m_x;
		clip_rect.y = rect.m_y;
		clip_rect.w = rect.m_x1 - rect.m_x;
		clip_rect.h = rect.m_y1 - rect.m_y;
		host_gui_set_clip(&clip_rect);
		//and draw
		host_gui_blit(texture, &srect, &drect);
	}
	return *this;
}

const Ctx &Ctx::panel(uint32_t col, bool filled, int32_t depth, int32_t x, int32_t y, int32_t w, int32_t h) const
{
	auto dark_col = darker(col);
	auto bright_col = brighter(col);
	auto abs_depth = std::abs(depth);
	if (filled)
	{
		//is filled
		set_color(col)
			.filled_box(x + abs_depth, y + abs_depth, w - 2 * abs_depth, h - 2 * abs_depth);
	}
	if (depth > 0)
	{
		//is out
		set_color(bright_col)
			.filled_box(x, y, w, abs_depth)
			.filled_box(x, y + abs_depth, abs_depth, h - abs_depth)
			.set_color(dark_col)
			.filled_box(x + abs_depth, y + h - abs_depth, w - abs_depth, abs_depth)
			.filled_box(x + w - abs_depth, y + abs_depth, abs_depth, h - 2 * abs_depth);
	}
	else if (depth < 0)
	{
		//is in
		set_color(dark_col)
			.filled_box(x, y, w - abs_depth, abs_depth)
			.filled_box(x, y + abs_depth, abs_depth, h - 2 * abs_depth)
			.set_color(bright_col)
			.filled_box(x, y + h - abs_depth, w, abs_depth)
			.filled_box(x + w - abs_depth, y + abs_depth, abs_depth, h - 2 * abs_depth);
	}
	return *this;
}
