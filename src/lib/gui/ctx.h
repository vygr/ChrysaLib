#ifndef CTX_H
#define CTX_H

#include <stdint.h>

class SDL_Renderer;
class View;
class Region;

//ctx class
//drawing context passed to widget draw methods
class Ctx
{
public:
	Ctx() {}
	const Ctx &set_color(uint32_t col) const;
	const Ctx &box(int32_t x, int32_t y, int32_t w, int32_t h) const;
	const Ctx &filled_box(int32_t x, int32_t y, int32_t w, int32_t h) const;
	const Ctx &panel(uint32_t col, bool filled, int32_t depth, int32_t x, int32_t y, int32_t w, int32_t h) const;
	uint32_t darker(uint32_t col) const;
	uint32_t brighter(uint32_t col) const;
	SDL_Renderer *m_renderer = nullptr;
	View *m_view = nullptr;
	Region *m_region = nullptr;
	int32_t m_x = 0;
	int32_t m_y = 0;
};

#endif
