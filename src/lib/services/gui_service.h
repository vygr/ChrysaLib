#ifndef GUI_H
#define GUI_H

#include "service.h"
#include <SDL.h>

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
	GUI_Service()
		: Service()
	{}
	void run() override;
	void composit();
	View *set_mouse_id();
	GUI_Service *quit(SDL_Event &e);
	GUI_Service *key_down(SDL_KeyboardEvent &e);
	GUI_Service *mouse_wheel(SDL_MouseWheelEvent &e);
	GUI_Service *mouse_button_down(SDL_MouseButtonEvent &e);
	GUI_Service *mouse_button_up(SDL_MouseButtonEvent &e);
	GUI_Service *mouse_motion(SDL_MouseMotionEvent &e);
	GUI_Service *window_event(SDL_WindowEvent &e);
	int32_t m_mouse_x = 0;
	int32_t m_mouse_y = 0;
	uint32_t m_mouse_buttons = 0;
	int64_t m_mouse_id = 0;
	std::shared_ptr<View> m_screen;
	SDL_Renderer *m_renderer = nullptr;
	SDL_Window *m_sdl_window = nullptr;
	SDL_Texture *m_texture = nullptr;
};

#endif
