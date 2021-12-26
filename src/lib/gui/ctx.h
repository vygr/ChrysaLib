#ifndef CTX_H
#define CTX_H

#include <stdint.h>

class SDL_Renderer;
class Region;

//ctx class
//drawing context passed to widget draw methods
class Ctx
{
public:
	Ctx() {}
	const Ctx &set_color(uint32_t col) const;
	const Ctx &box(int x, int y, int w, int h) const;
	const Ctx &filled_box(int x, int y, int w, int h) const;
	const Ctx &panel(uint32_t col, bool filled, int depth, int x, int y, int w, int h) const;
	uint32_t darker(uint32_t col) const;
	uint32_t brighter(uint32_t col) const;
	SDL_Renderer *m_renderer = nullptr;
	Region *m_region = nullptr;
	int m_x = 0;
	int m_y = 0;
};

#endif
