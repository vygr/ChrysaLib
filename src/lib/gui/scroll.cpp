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
		auto vval = (int64_t)0;
		auto hval = (int64_t)0;
		auto cs = m_child->get_size();
		if (m_vslider)
		{
			auto mo = (int64_t)std::max(0, cs.m_h - (m_h - sh));
			vval = m_vslider->get_long_prop("value");
			vval = std::max((int64_t)0, std::min(vval, mo));
			m_vslider->def_prop("value", std::make_shared<Property>(vval));
			m_vslider->def_prop("maximum", std::make_shared<Property>(mo));
			m_vslider->def_prop("portion", std::make_shared<Property>(m_h - sh));
		}
		if (m_hslider)
		{
			auto mo = (int64_t)std::max(0, cs.m_w - (m_w - sw));
			hval = m_hslider->get_long_prop("value");
			hval = std::max((int64_t)0, std::min(hval, mo));
			m_hslider->def_prop("value", std::make_shared<Property>(hval));
			m_hslider->def_prop("maximum", std::make_shared<Property>(mo));
			m_hslider->def_prop("portion", std::make_shared<Property>(m_w - sw));
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
		auto value = m_hslider->get_long_prop("value");
		m_hslider->def_prop("value", std::make_shared<Property>(value + 16 * event_struct->m_x));
	}
	if (m_vslider)
	{
		auto value = m_vslider->get_long_prop("value");
		m_vslider->def_prop("value", std::make_shared<Property>(value - 16 * event_struct->m_y));
	}
	layout()->dirty_all();
	return this;
}
