#include "progress.h"
#include "ctx.h"

view_size Progress::pref_size()
{
	return view_size{256, 10};
}

Progress *Progress::draw(const Ctx &ctx)
{
	//allready locked by GUI thread
	auto col = (uint32_t)get_long_prop("color");
	auto value = get_long_prop("value");
	auto max = get_long_prop("maximum");
	auto gap = value * (m_w - 2) / max;
	auto dark = ctx.darker(col);
	auto darker = ctx.darker(dark);
	//border
	ctx.set_color(col).box(0, 0, m_w, m_h)
	//middle left
	.set_color(dark).filled_box(1, 1, gap, m_h - 2)
	//middle right
	.set_color(darker).filled_box(gap + 1, 1, m_w - 2 - gap, m_h - 2);
	return this;
}
