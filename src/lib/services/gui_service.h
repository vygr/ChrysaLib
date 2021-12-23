#ifndef GUI_H
#define GUI_H

#include "service.h"

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
	unsigned int m_gui_flags;
	bool m_dirty_flag = true;
};

#endif
