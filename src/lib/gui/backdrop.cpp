#include "backdrop.h"
#include "ctx.h"

Backdrop *Backdrop::draw(Ctx *ctx)
{
	auto col = get_prop("color")->get_int();
	ctx->set_color(col);
	ctx->filled_box(m_x, m_y, m_w, m_h);
	return this;
}
