#include "gui_service.h"
#include "../gui/region.h"
#include "../gui/ctx.h"
#include "../gui/backdrop.h"
#include <iostream>
#include <sstream>
#include <SDL.h>

//////
// gui
//////

void GUI_Service::run()
{
	//get my mailbox address, id was allocated in the constructor
	auto mbox = m_router.validate(m_net_id);
	auto entry = m_router.declare(m_net_id, "gui", "GUI_Service v0.1");

	m_screen = std::make_shared<Backdrop>(0, 0, 1280, 960);
	auto color = std::make_shared<Property>();
	color->set_int(0xff0000ff);
	m_screen->def_prop("color", color);
	auto screen_w = m_screen->m_w;
	auto screen_h = m_screen->m_h;

	//init SDL
	SDL_SetMainReady();
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	//create window
	SDL_Window *window = SDL_CreateWindow("GUI Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	//hide host cursor
	//SDL_ShowCursor(0);
	//renderer
	m_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	SDL_Texture *texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, screen_w, screen_h);
	//set blend mode
	SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

	//event loop
	while (m_running)
	{
		auto msg = mbox->poll();
		if (msg)
		{
			auto evt = (Event*)msg->begin();
			switch (evt->m_evt)
			{
			case evt_add_front_view:
			{
				//add view to screen
				auto body_struct = (Event_add_front_view*)msg->begin();
				break;
			}
			case evt_add_back_view:
			{
				//add view to screen
				auto body_struct = (Event_add_back_view*)msg->begin();
				break;
			}
			case evt_sub_view:
			{
				//sub view from screen
				auto body_struct = (Event_sub_view*)msg->begin();
				break;
			}
			default:
				break;
			}
		}

		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				m_running = false;
			}
			if (e.type == SDL_KEYDOWN)
			{
				m_running = false;
			}
			if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				m_running = false;
			}
		}

		if (m_gui_flags)
		{
			//resize back buffer and redraw in full
			SDL_DestroyTexture (texture);
			SDL_CreateTexture (m_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, screen_w, screen_h);
			SDL_SetRenderTarget (m_renderer, texture);
			m_gui_flags = 0;
		}
		if (m_dirty_flag)
		{
			SDL_SetRenderTarget (m_renderer, 0);
			composit();
			SDL_RenderCopy (m_renderer, texture, 0, 0);
			SDL_RenderPresent(m_renderer);
			m_dirty_flag = false;
		}

		//frame polling loop
		std::this_thread::sleep_for(std::chrono::milliseconds(GUI_FRAME_RATE));
	}

	//quit SDL
	SDL_DestroyWindow(window);
	SDL_Quit();

	//forget myself
	m_router.forget(entry);
}

void GUI_Service::composit()
{
	auto screen_w = m_screen->m_w;
	auto screen_h = m_screen->m_h;
	Region region;
	Ctx ctx;
	ctx.m_x = 0;
	ctx.m_y = 0;
	ctx.m_renderer = m_renderer;
	ctx.m_region = &region;
	region.paste_rect(Rect(0, 0, screen_w, screen_h));
	m_screen->draw(&ctx);
	for (auto &view : m_screen->m_children)
	{
		view->draw(&ctx);
	}
}
