#ifndef CTX_H
#define CTX_H

class SDL_Renderer;
class Region;

//ctx class
//drawing context passed to widget draw methods
class Ctx
{
public:
	Ctx()
	{}
	Ctx *set_color(unsigned int col);
	Ctx *box(int x, int y, int w, int h);
	Ctx *filled_box(int x, int y, int w, int h);
	Ctx *panel(unsigned int col, bool filled, int depth, int w, int h);
	SDL_Renderer *m_renderer = nullptr;
	Region *m_region;
	int m_x = 0;
	int m_y = 0;
};

#endif
