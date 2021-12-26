#include "button.h"
#include "ctx.h"

Button *Button::mouse_down(const std::shared_ptr<Msg> &event)
{
	auto event_struct = (View::Event_ev_msg_mouse*)&*(event->begin());
	m_state = -1;
	layout()->dirty_all();
	return this;
}

Button *Button::mouse_up(const std::shared_ptr<Msg> &event)
{
	auto event_struct = (View::Event_ev_msg_mouse*)&*(event->begin());
	if (m_state != 1)
	{
		m_state = 1;
		layout()->dirty_all()->emit();
	}
	return this;
}

Button *Button::mouse_move(const std::shared_ptr<Msg> &event)
{
	auto event_struct = (View::Event_ev_msg_mouse*)&*(event->begin());
	auto state = 1;
	if (event_struct->m_rx >= 0
		&& event_struct->m_ry >= 0
		&& event_struct->m_rx < m_w
		&& event_struct->m_ry < m_h) state = -1;
	if (state != m_state)
	{
		m_state = state;
		layout()->dirty_all();
	}
	return this;
}
