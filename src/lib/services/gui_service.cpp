#include "gui_service.h"
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

	//init SDL
	SDL_SetMainReady();
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	//create window
	SDL_Window *window = SDL_CreateWindow("GUI Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 960, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	//hide host cursor
	//SDL_ShowCursor(0);
	//renderer
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, 1280, 960);
	//set blend mode
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

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
			SDL_CreateTexture (renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, 1280, 960);
			SDL_SetRenderTarget (renderer, texture);
			m_gui_flags = 0;
		}
		if (m_dirty_flag)
		{
			SDL_SetRenderTarget (renderer, 0);
			composit();
			SDL_RenderCopy (renderer, texture, 0, 0);
			SDL_RenderPresent(renderer);
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
	
}
