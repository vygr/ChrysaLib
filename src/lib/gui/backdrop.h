#ifndef BACKDROP_H
#define BACKDROP_H

#include "view.h"

//backdrop widget
class Backdrop : public View
{
public:
	Backdrop(int x, int y, int w, int h)
		: View(x, y, w, h)
	{}
	Backdrop *draw(Ctx *ctx);
};

#endif
