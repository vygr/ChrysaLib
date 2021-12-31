#include "text.h"
#include "ctx.h"
#include "colors.h"

view_size Text::pref_size()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	//auto font = get_string_prop("font");
	auto text = get_string_prop("text");
	auto len = (int32_t)text.size();
	if (!len) len = 1;
	return view_size{len * 8, 16};
}

Text *Text::draw(const Ctx &ctx)
{
	//allready locked by GUI thread
	//auto font = get_string_prop("font");
	//auto text = get_string_prop("text");
	auto col = (uint32_t)get_long_prop("color");
	//auto ink_col = (uint32_t)get_long_prop("ink_color");
	ctx.set_color(col).filled_box(0, 0, m_w, m_h)
		.set_color(argb_red).box(0, 0, m_w, m_h);
	return this;
}
