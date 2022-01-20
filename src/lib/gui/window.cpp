#include "window.h"
#include "colors.h"
#include "font.h"
#include "ctx.h"
#include <iostream>

Window::Window()
	: View()
{
	def_props({
		{"is_window", 0},
		});
}

Window *Window::add_child(std::shared_ptr<View> child)
{
	std::lock_guard<std::recursive_mutex> l(m_mutex);
	if (m_child) m_child->sub();
	if (child) add_back(child);
	m_child = child;
	return this;
}

view_size Window::pref_size()
{
	std::lock_guard<std::recursive_mutex> l(m_mutex);
	auto border = get_long_prop("border");
	auto shadow = get_long_prop("shadow");
	view_size s;
	if (m_child) s = m_child->pref_size();
	s.m_w += 2 * (shadow + border);
	s.m_h += 2 * (shadow + border);
	return s;
}

Window *Window::layout()
{
	std::lock_guard<std::recursive_mutex> l(m_mutex);
	auto col = (uint32_t)get_long_prop("color");
	auto border = get_long_prop("border");
	auto shadow = get_long_prop("shadow");
	//position any child
	if (m_child) m_child->change(border + shadow, border + shadow,
		m_w - 2 * (shadow + border), m_h - 2 * (shadow + border));
	//adjust window transparency details based on color and shadow
	if ((col >> 24) == 0xff)
	{
		if (!shadow) set_flags(view_flag_opaque, view_flag_opaque);
		else clr_opaque()->add_opaque(shadow, shadow, m_w - 2 * shadow, m_h - 2 * shadow);
	}
	return this;
}

Window *Window::draw(const Ctx &ctx)
{
	//allready locked by GUI thread
	auto col = (uint32_t)get_long_prop("color");
	auto border = get_long_prop("border");
	auto shadow = get_long_prop("shadow");
	ctx.panel(col, true, border, shadow, shadow, m_w - 2 * shadow, m_h - 2 * shadow);
	col = 0x80000000;
	while (--shadow >= 0 && col != 0)
	{
		ctx.set_color(col)
			.box(shadow, shadow, m_w - 2 * shadow, m_h - 2 * shadow);
		col = 0xff000000 & ((col >> 1) + (col >> 4));
	}
	return this;
}

drag_mode Window::get_drag_mode(int32_t rx, int32_t ry)
{
	drag_mode mode;
	auto border = get_long_prop("border");
	auto shadow = get_long_prop("shadow");
	if (rx < (border + shadow))
	{
		mode.m_mode |= 1;
		mode.m_x = rx;
	}
	if (ry < (border + shadow))
	{
		mode.m_mode |= 2;
		mode.m_y = ry;
	}
	if (rx >= (m_w - border - shadow))
	{
		mode.m_mode |= 4;
		mode.m_x = rx - m_w;
	}
	if (ry >= (m_h - border - shadow))
	{
		mode.m_mode |= 8;
		mode.m_y = ry - m_h;
	}
	return mode;
}

Window *Window::mouse_down(const std::shared_ptr<Msg> &event)
{
	std::lock_guard<std::recursive_mutex> l(m_mutex);
	auto event_body = (View::Event_mouse*)event->begin();
	m_mode = get_drag_mode(event_body->m_rx, event_body->m_ry);
	return this;
}

Window *Window::mouse_move(const std::shared_ptr<Msg> &event)
{
	std::lock_guard<std::recursive_mutex> l(m_mutex);
	auto event_body = (View::Event_mouse*)event->begin();
	auto ax = event_body->m_x;
	auto ay = event_body->m_y;
	auto bounds = get_bounds();
	auto size = pref_size();
	bounds.m_w += bounds.m_x;
	bounds.m_h += bounds.m_y;
	if ((m_mode.m_mode & 1) != 0) bounds.m_x = std::min(ax - m_mode.m_x, bounds.m_w - size.m_w);
	if ((m_mode.m_mode & 2) != 0) bounds.m_y = std::min(ay - m_mode.m_y, bounds.m_h - size.m_h);
	if ((m_mode.m_mode & 4) != 0) bounds.m_w = std::max(ax - m_mode.m_x, bounds.m_x + size.m_w);
	if ((m_mode.m_mode & 8) != 0) bounds.m_h = std::max(ay - m_mode.m_y, bounds.m_y + size.m_h);
	change_dirty(bounds.m_x, bounds.m_y, bounds.m_w - bounds.m_x, bounds.m_h - bounds.m_y)
		->emit();
	return this;
}

Window *Window::event(const std::shared_ptr<Msg> &event)
{
	auto event_body = (View::Event*)event->begin();
	auto target = find_id(event_body->m_evt);
	auto type = event_body->m_type;
	if (target)
	{
		if (type == ev_type_mouse)
		{
			//so what state are we in ?
			auto button_event_body = (View::Event_mouse*)event->begin();
			auto buttons = button_event_body->m_buttons;
			if (m_last_buttons)
			{
				//was down previously
				if (buttons)
				{
					//is down now, so move
					target->mouse_move(event);
				}
				else
				{
					//is not down now, so release
					m_last_buttons = 0;
					target->mouse_up(event);
				}
			}
			else
			{
				//was not down previously
				if (buttons)
				{
					//is down now, so first down
					m_last_buttons = buttons;
					target->mouse_down(event);
				}
				else
				{
					//is not down now, so hover
					target->mouse_hover(event);
				}
			}
		}
		else if (type == ev_type_key)
		{
			target->key_down(event);
			target->key_up(event);
		}
		else if (type == ev_type_wheel)
		{
			auto wheel = target;
			while (wheel && !wheel->got_prop("has_wheel")) { wheel = wheel->m_parent; }
			if (wheel) wheel->mouse_wheel(event);
		}
		else if (type == ev_type_enter)
		{
			target->mouse_enter(event);
		}
		else if (type == ev_type_exit)
		{
			target->mouse_exit(event);
		}
		else if (type == ev_type_action)
		{
			target->action(event);
		}
	}
	return this;
}
