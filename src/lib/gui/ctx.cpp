#include "ctx.h"
#include "region.h"
#include <SDL.h>

Ctx *Ctx::set_color(unsigned int col)
{
	uint8_t a = (col >> 24) & 0xff;
	uint8_t r = (col >> 16) & 0xff;
	uint8_t g = (col >> 8) & 0xff;
	uint8_t b = col & 0xff;
	SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
	return this;
}

Ctx *Ctx::box(int x, int y, int w, int h)
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
		SDL_RenderSetClipRect(m_renderer, &clip_rect);
		//and draw
		SDL_RenderDrawRect(m_renderer, &drect);
	}
	return this;
}

Ctx *Ctx::filled_box(int x, int y, int w, int h)
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
		SDL_RenderSetClipRect(m_renderer, &clip_rect);
		//and draw
		SDL_RenderFillRect(m_renderer, &drect);
	}
	return this;
}

Ctx *Ctx::panel(unsigned int col, bool filled, int depth, int w, int h)
{
	set_color(col);
	filled_box(0, 0, w, h);
	return this;
}
