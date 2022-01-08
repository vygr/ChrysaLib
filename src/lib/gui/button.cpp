#include "button.h"
#include "ctx.h"

Button::Button()
	: Label()
{}

Button *Button::layout()
{
	std::lock_guard<std::recursive_mutex> l(m_mutex);
	auto border = get_long_prop("border");
	auto pos = border;
	if (m_state != 1) pos *= 2;
	m_flow->change(pos, pos, m_w - border * 2, m_h - border * 2)->layout();
	set_flags(view_flag_opaque, view_flag_opaque);
	return this;
}

Button *Button::draw(const Ctx &ctx)
{
	//allready locked by GUI thread
	auto col = (uint32_t)get_long_prop("color");
	auto border = get_long_prop("border");
	ctx.panel(col, true, border * m_state, 0, 0, m_w, m_h);
	return this;
}

Button *Button::mouse_down(const std::shared_ptr<Msg> &event)
{
	std::lock_guard<std::recursive_mutex> l(m_mutex);
	m_state = -1;
	layout()->dirty_all();
	return this;
}

Button *Button::mouse_up(const std::shared_ptr<Msg> &event)
{
	std::lock_guard<std::recursive_mutex> l(m_mutex);
	if (m_state != 1)
	{
		m_state = 1;
		layout()->dirty_all()->emit();
	}
	return this;
}

Button *Button::mouse_move(const std::shared_ptr<Msg> &event)
{
	std::lock_guard<std::recursive_mutex> l(m_mutex);
	auto event_body = (View::Event_mouse*)&*(event->begin());
	auto state = 1;
	if (event_body->m_rx >= 0
		&& event_body->m_ry >= 0
		&& event_body->m_rx < m_w
		&& event_body->m_ry < m_h) state = -1;
	if (state != m_state)
	{
		m_state = state;
		layout()->dirty_all();
	}
	return this;
}
