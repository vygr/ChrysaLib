#include "slider.h"
#include "colors.h"
#include "ctx.h"

Slider::Slider()
	: View()
{
	def_prop("value", std::make_shared<Property>(0))
	->def_prop("maximum", std::make_shared<Property>(0))
	->def_prop("minimum", std::make_shared<Property>(0))
	->def_prop("portion", std::make_shared<Property>(0));
}

view_size Slider::pref_size()
{
	return view_size{10, 10};
}

Slider *Slider::draw(const Ctx &ctx)
{
	//allready locked by GUI thread
	auto col = (uint32_t)get_long_prop("color");
	auto value = (int)get_long_prop("value");
	auto portion = (int)get_long_prop("portion");
	auto max = (int)get_long_prop("maximum");
	auto dark = ctx.darker(col);
	auto bright = ctx.darker(col);
	//border
	ctx.set_color(argb_black).box(0, 0, m_w, m_h)
	//middle
	.set_color(dark).filled_box(1, 1, m_w - 2, m_h - 2)
	//elevator
	.set_color(m_state ? bright : col);
	if (m_w > m_h)
	{
		auto w = m_w - 2;
		if (max > 0)
		{
			portion = portion * w / (max + portion);
			w = value * (w - portion) * max;
		}
		else
		{
			portion = w;
			w = 0;
		}
		ctx.filled_box(w + 1, 1, portion, m_h - 2);
	}
	else
	{
		auto h = m_h - 2;
		if (max > 0)
		{
			portion = portion * h / (max + portion);
			h = value * (h - portion) * max;
		}
		else
		{
			portion = h;
			h = 0;
		}
		ctx.filled_box(1, h + 1, m_w -2, portion);
	}
	return this;
}

Slider *Slider::mouse_down(const std::shared_ptr<Msg> &event)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto event_struct = (View::Event_mouse*)&*(event->begin());
	m_old_value = (int)get_long_prop("value");
	m_state = 1;
	m_down_xy = m_w > m_h ? event_struct->m_rx : event_struct->m_ry;
	dirty_all()->mouse_move(event);
	return this;
}

Slider *Slider::mouse_up(const std::shared_ptr<Msg> &event)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto event_struct = (View::Event_mouse*)&*(event->begin());
	if (m_state != 0)
	{
		m_state = 0;
		dirty_all();
	}
	return this;
}

Slider *Slider::mouse_move(const std::shared_ptr<Msg> &event)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto event_struct = (View::Event_mouse*)&*(event->begin());
	auto value_prop = get_prop("value");
	auto value = 0;
	if (value_prop) value = (int)value_prop->get_long();
	auto portion = (int)get_long_prop("portion");
	auto max = (int)get_long_prop("maximum");
	auto state = 1;
	if (event_struct->m_rx >= 0
		&& event_struct->m_ry >= 0
		&& event_struct->m_rx < m_w
		&& event_struct->m_ry < m_h) state = 0;
	auto new_value = m_w > m_h ?
		(event_struct->m_rx - m_down_xy) * (max + portion) / m_w:
		(event_struct->m_ry - m_down_xy) * (max + portion) / m_h;
	new_value = (std::max(0, std::min(max, (new_value + m_old_value))));
	if (new_value != value)
	{
		if (value_prop) value_prop->set_long(new_value);
		dirty_all()->emit();
	}
	if (state != m_state)
	{
		m_state = state;
		dirty_all();
	}
	return this;
}
