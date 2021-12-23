#include "backdrop.h"
#include "property.h"
#include "ctx.h"

Backdrop::Backdrop(int x, int y, int w, int h)
	: View(x, y, w, h)
{
	auto spaceing = std::make_shared<Property>();
	auto color = std::make_shared<Property>();
	auto ink_color = std::make_shared<Property>();
	auto style = std::make_shared<Property>();
	spaceing->set_int(32);
	color->set_int(0xff000000);
	ink_color->set_int(0xffffffff);
	style->set_str("grid");
	def_prop("color", color);
	def_prop("ink_color", ink_color);
	def_prop("spaceing", spaceing);
	def_prop("style", style);
}

Backdrop *Backdrop::draw(Ctx *ctx)
{
	auto col = get_prop("color")->get_int();
	auto ink_col = get_prop("ink_color")->get_int();
	auto spaceing = get_prop("spaceing")->get_int();
	auto style = get_prop("style")->get_str();
	ctx->set_color(col);
	ctx->filled_box(m_x, m_y, m_w, m_h);
	if (style == "grid")
	{
		ctx->set_color(ink_col);
		for (int x = (((m_w >> 1) % spaceing) - spaceing); x < m_w; x += spaceing)
		{
			ctx->filled_box(x, 0, 1, m_h);
		}
		for (int y = (((m_h >> 1) % spaceing) - spaceing); y < m_h; y += spaceing)
		{
			ctx->filled_box(0, y, m_w, 1);
		}
	}
	else if (style == "axis")
	{
		ctx->set_color(ink_col);
		ctx->filled_box((m_w >> 1), 0, 1, m_h);
		ctx->filled_box(0, (m_h >> 1), m_w, 1);
		auto x = ((m_w >> 1) % spaceing) - spaceing;
		auto y = (m_h >> 1) - (spaceing / 2);
		for (; x < m_w; x += spaceing)
		{
			ctx->filled_box(x, y, 1, spaceing);
		}
		for (; x < m_w; x += spaceing)
		{
			ctx->filled_box(x, y, 1, spaceing);
		}
		y = ((m_h >> 1) % spaceing) - spaceing;
		x = (m_w >> 1) - (spaceing / 2);
		for (; y < m_h; y += spaceing)
		{
			ctx->filled_box(x, y, spaceing, 1);
		}
	}
	else if (style == "lines")
	{
		ctx->set_color(ink_col);
		for (int y = (((m_h >> 1) % spaceing) - spaceing); y < m_h; y += spaceing)
		{
			ctx->filled_box(0, y, m_w, 1);
		}
	}
	return this;
}
