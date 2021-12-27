#include "scroll.h"

Scroll::Scroll(int flags)
	: View()
{
	if ((flags & scroll_flag_vertical) != 0)
	{
		m_vslider = std::make_shared<Slider>();
		m_vslider->connect(m_id);
		add_front(m_vslider);
	}
	if ((flags & scroll_flag_horizontal) != 0)
	{
		m_hslider = std::make_shared<Slider>();
		m_hslider->connect(m_id);
		add_front(m_hslider);
	}
}

view_size Scroll::pref_size()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto mw = (int)got_long_prop("min_width");
	auto mh = (int)got_long_prop("min_height");
	if (m_vslider) mw = mw + m_vslider->pref_size().m_w;
	if (m_hslider) mh = mh + m_hslider->pref_size().m_h;
	return view_size{mw, mh};
}

Scroll *Scroll::add_child(std::shared_ptr<View> child)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	if (m_child) m_child->sub();
	if (child) add_back(child);
	m_child = child;
	return this;
}

Scroll *Scroll::layout()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	//position any sliders
	auto sw = 0;
	auto sh = 0;
	if (m_vslider)
	{
		sw = m_vslider->pref_size().m_w;
		m_vslider->change(m_w - sw, 0, sw, m_h);
	}
	if (m_hslider)
	{
		sh = m_hslider->pref_size().m_w;
		m_hslider->change(0, m_h - sh, m_w - sw, sh);
	}
	//position any child
	if (m_child)
	{
		auto vval = 0;
		auto hval = 0;
		auto cs = m_child->get_size();
		if (m_vslider)
		{
			auto value_prop = m_vslider->get_prop("value");
			auto max_prop = m_vslider->get_prop("maximum");
			auto portion_prop = m_vslider->get_prop("portion");
			auto mo = std::max(0, cs.m_h - (m_h - sh));
			if (max_prop && value_prop && portion_prop)
			{
				vval = std::max(0, std::min((int)value_prop->get_long(), mo));
				max_prop->set_long(mo);
				portion_prop->set_long(m_h - sh);
				value_prop->set_long(vval);
			}
		}
		if (m_hslider)
		{
			auto value_prop = m_hslider->get_prop("value");
			auto max_prop = m_hslider->get_prop("maximum");
			auto portion_prop = m_hslider->get_prop("portion");
			auto mo = std::max(0, cs.m_w - (m_w - sw));
			if (max_prop && value_prop && portion_prop)
			{
				hval = std::max(0, std::min((int)value_prop->get_long(), mo));
				max_prop->set_long(mo);
				portion_prop->set_long(m_w - sw);
				value_prop->set_long(hval);
			}
		}
		m_child->m_x = -hval;
		m_child->m_y = -vval;
		m_child->m_w = cs.m_w;
		m_child->m_h = cs.m_h;
	}
	return this;
}

Scroll *Scroll::action(const std::shared_ptr<Msg> &event)
{
	layout()->dirty_all();
	return this;
}

Scroll *Scroll::mouse_wheel(const std::shared_ptr<Msg> &event)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto event_struct = (View::Event_wheel*)&*(event->begin());
	if (m_hslider)
	{
		auto value_prop = m_hslider->get_prop("value");
		if (value_prop) value_prop->set_long(value_prop->get_long() + 16 * event_struct->m_x);
	}
	if (m_vslider)
	{
		auto value_prop = m_vslider->get_prop("value");
		if (value_prop) value_prop->set_long(value_prop->get_long() - 16 * event_struct->m_y);
	}
	layout()->dirty_all();
	return this;
}
