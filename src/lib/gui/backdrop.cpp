#include "backdrop.h"
#include "property.h"
#include "ctx.h"
#include "colors.h"

Backdrop::Backdrop()
	: View()
{
	def_prop("color", std::make_shared<Property>(argb_black))
	->def_prop("ink_color", std::make_shared<Property>(argb_white))
	->def_prop("spaceing", std::make_shared<Property>(32))
	->def_prop("style", std::make_shared<Property>("grid"));
}

Backdrop *Backdrop::draw(const Ctx &ctx)
{
	//allready locked by GUI thread
	auto col = (uint32_t)get_long_prop("color");
	auto ink_col = (uint32_t)get_long_prop("ink_color");
	auto spaceing = get_long_prop("spaceing");
	auto style = get_string_prop("style");
	ctx.set_color(col)
		.filled_box(0, 0, m_w, m_h);
	if (style == "grid")
	{
		ctx.set_color(ink_col);
		for (int x = (((m_w >> 1) % spaceing) - spaceing); x < m_w; x += spaceing)
		{
			ctx.filled_box(x, 0, 1, m_h);
		}
		for (int y = (((m_h >> 1) % spaceing) - spaceing); y < m_h; y += spaceing)
		{
			ctx.filled_box(0, y, m_w, 1);
		}
	}
	else if (style == "axis")
	{
		ctx.set_color(ink_col)
			.filled_box((m_w >> 1), 0, 1, m_h)
			.filled_box(0, (m_h >> 1), m_w, 1);
		auto x = ((m_w >> 1) % spaceing) - spaceing;
		auto y = (m_h >> 1) - (spaceing / 2);
		for (; x < m_w; x += spaceing)
		{
			ctx.filled_box(x, y, 1, spaceing);
		}
		for (; x < m_w; x += spaceing)
		{
			ctx.filled_box(x, y, 1, spaceing);
		}
		y = ((m_h >> 1) % spaceing) - spaceing;
		x = (m_w >> 1) - (spaceing / 2);
		for (; y < m_h; y += spaceing)
		{
			ctx.filled_box(x, y, spaceing, 1);
		}
	}
	else if (style == "lines")
	{
		ctx.set_color(ink_col);
		for (int y = (((m_h >> 1) % spaceing) - spaceing); y < m_h; y += spaceing)
		{
			ctx.filled_box(0, y, m_w, 1);
		}
	}
	return this;
}
