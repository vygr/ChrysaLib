#ifndef GUI_H
#define GUI_H

#include "service.h"

class View;
class SDL_Renderer;

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
	GUI_Service(Router &router)
		: Service(router)
	{}
	void run() override;
	void composit();
	uint32_t m_gui_flags = 0;
	std::shared_ptr<View> m_screen;
	SDL_Renderer *m_renderer = nullptr;
};

#endif
