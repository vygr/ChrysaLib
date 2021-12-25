#include "window.h"
#include "colors.h"
#include "ctx.h"

Window::Window()
	: View()
{
	def_prop("color", std::make_shared<Property>(argb_grey12));
	def_prop("border", std::make_shared<Property>(2));
	def_prop("shadow", std::make_shared<Property>(5));
}

Window *Window::add_child(std::shared_ptr<View> child)
{
	if (m_child) m_child->sub();
	if (child) add_back(child);
	m_child = child;
	return this;
}

view_size Window::get_pref_size()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto border = (int)get_long_prop("border");
	auto shadow = (int)get_long_prop("shadow");
	view_size s;
	if (m_child) s = m_child->get_pref_size();
	s.m_w = (s.m_w + 2 * (shadow + border));
	s.m_h = (s.m_h + 2 * (shadow + border));
	return s;
}

Window *Window::layout()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto border = (int)get_long_prop("border");
	auto shadow = (int)get_long_prop("shadow");
	auto col = (uint32_t)get_long_prop("color");
	//position any child
	auto w = m_w - 2 * (shadow + border);
	auto h = m_h - 2 * (shadow + border);
	if (m_child) m_child->change(border + shadow, border + shadow, w, h);
	//adjust window transparency details based on color and shadow
	if ((col >> 24) == 0xff)
	{
		if (shadow == 0) set_flags(view_flag_opaque, view_flag_opaque);
		else clr_opaque()->add_opaque(Rect(border + shadow, border + shadow, w, h));
	}
	return this;
}

Window *Window::draw(Ctx *ctx)
{
	//allready locked by GUI thread
	auto col = (uint32_t)get_long_prop("color");
	auto border = (int)get_long_prop("border");
	auto shadow = (int)get_long_prop("shadow");
	ctx->panel(col, true, border, shadow, shadow, m_w - 2 * shadow, m_h - 2 * shadow);
	col = 0x80000000;
	while (shadow > 0 && col != 0)
	{
		ctx->set_color(col);
		ctx->box(shadow, shadow, m_w - 2 * shadow, m_h - 2 * shadow);
		col = (0xff000000 & (col >> 1) + (col >> 4));
		--shadow;
	}
	return this;
}
