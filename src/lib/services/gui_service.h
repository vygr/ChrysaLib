#ifndef GUI_H
#define GUI_H

#include "service.h"
#include "../gui/view.h"
#include "../../host/sdl_dummy.h"

//gui service
class GUI_Service : public Service
{
public:
	enum
	{
		evt_exit, //must be first !
		evt_add_front,
		evt_add_back,
		evt_sub,
		evt_locate,
	};
	struct Event_add_front : public Event
	{
		Net_ID m_reply;
		std::shared_ptr<View> m_view;
	};
	struct Event_add_back : public Event
	{
		Net_ID m_reply;
		std::shared_ptr<View> m_view;
	};
	struct Event_sub : public Event
	{
		Net_ID m_reply;
		std::shared_ptr<View> m_view;
	};
	struct Event_locate : public Event
	{
		Net_ID m_reply;
		int32_t m_w;
		int32_t m_h;
		int32_t m_pos;
	};
	struct locate_reply
	{
		view_bounds m_bounds;
	};
	GUI_Service()
		: Service()
	{}
	void run() override;
	void composite();
	View *set_mouse_id();
	GUI_Service *quit(const SDL_Event &e);
	GUI_Service *key_down(const SDL_KeyboardEvent &e);
	GUI_Service *mouse_wheel(const SDL_MouseWheelEvent &e);
	GUI_Service *mouse_button_down(const SDL_MouseButtonEvent &e);
	GUI_Service *mouse_button_up(const SDL_MouseButtonEvent &e);
	GUI_Service *mouse_motion(const SDL_MouseMotionEvent &e);
	GUI_Service *window_event(const SDL_WindowEvent &e);
	int32_t m_mouse_x = 0;
	int32_t m_mouse_y = 0;
	uint32_t m_mouse_buttons = 0;
	int64_t m_mouse_id = 0;
	std::shared_ptr<View> m_screen;
};

#endif
