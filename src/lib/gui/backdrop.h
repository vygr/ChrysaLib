#ifndef BACKDROP_H
#define BACKDROP_H

#include "view.h"

//backdrop widget
class Backdrop : public View
{
public:
	Backdrop(int x, int y, int w, int h);
	Backdrop *draw(Ctx *ctx) override;
};

#endif
