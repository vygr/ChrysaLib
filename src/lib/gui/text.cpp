#include "text.h"
#include "ctx.h"
#include "colors.h"

Text::Text()
	: View()
{
	def_prop("color", std::make_shared<Property>(0));
}

view_size Text::pref_size()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto font = get_font_prop("font");
	auto text = get_string_prop("text");
	if (font)
	{
		auto info = font->glyph_info(text);
		auto size = font->glyph_bounds(info);
		return view_size{(int32_t)size.m_w, (int32_t)size.m_h};
	}
	return view_size{0, 0};
}

Text *Text::draw(const Ctx &ctx)
{
	//allready locked by GUI thread
	auto font = get_font_prop("font");
	auto text = get_string_prop("text");
	auto ink_col = (uint32_t)get_long_prop("ink_color");
	if (font)
	{
		auto sym_texture = font->sym_texture(text);
		if (sym_texture)
		{
			ctx.blit(sym_texture->m_handle, ink_col, 0, (m_h - sym_texture->m_h) >> 1, sym_texture->m_w, sym_texture->m_h);
		}
	}
	return this;
}
