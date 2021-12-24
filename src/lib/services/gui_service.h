#ifndef GUI_H
#define GUI_H

#include "service.h"

enum
{
	ev_type_mouse,
	ev_type_key,
	ev_type_action,
	ev_type_gui,
	ev_type_wheel,
	ev_type_enter,
	ev_type_exit
};

struct Event_ev_msg
{
	unsigned long target_id;
	unsigned long type;
};

struct Event_ev_msg_mouse : public Event_ev_msg
{
	unsigned int buttons;
	unsigned int count;
	int x;
	int y;
	int rx;
	int ry;
};

struct Event_ev_msg_wheel : public Event_ev_msg
{
	unsigned int direction;
	int x;
	int y;
};

struct Event_ev_msg_key : public Event_ev_msg
{
	unsigned int keycode;
	unsigned int key;
	unsigned int mod;
};

struct Event_ev_msg_action : public Event_ev_msg
{
	unsigned long source_id;
};

struct Event_ev_msg_gui : public Event_ev_msg
{
};

struct Event_ev_msg_enter : public Event_ev_msg
{
};

struct Event_ev_msg_exit : public Event_ev_msg
{
};

class View;
class SDL_Renderer;

//gui service
class GUI_Service : public Service
{
public:
	enum
	{
		evt_exit, //must be first !
		evt_add_front_view,
		evt_add_back_view,
		evt_sub_view,
	};
	struct Event_add_front_view : public Event
	{
		Net_ID m_reply;
	};
	struct Event_add_back_view : public Event
	{
		Net_ID m_reply;
	};
	struct Event_sub_view : public Event
	{
		Net_ID m_reply;
	};
	GUI_Service(Router &router)
		: Service(router)
	{}
	void run() override;
	void composit();
	unsigned int m_gui_flags = 0;
	bool m_dirty_flag = true;
	std::shared_ptr<View> m_screen;
	SDL_Renderer *m_renderer = nullptr;
};

#endif
