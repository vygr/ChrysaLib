#include "backdrop.h"
#include "ctx.h"

Backdrop *Backdrop::draw(Ctx *ctx)
{
	ctx->set_color(0xff00ff00);
	ctx->filled_box(m_x, m_y, m_w, m_h);
	return this;
}
