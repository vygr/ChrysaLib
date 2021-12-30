#include "gui_service.h"
#include "gui_task.h"
#include "../gui/region.h"
#include "../gui/ctx.h"
#include "../gui/backdrop.h"
#include "../gui/colors.h"
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

	m_screen = std::make_shared<Backdrop>();
	m_screen->def_prop("color", std::make_shared<Property>(argb_grey2))
		->def_prop("ink_color", std::make_shared<Property>(argb_grey1))
		->change(0, 0, 1280, 960)->dirty_all();

	//init SDL
	SDL_SetMainReady();
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	//create window
	SDL_Window *sdl_window = SDL_CreateWindow("GUI Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_screen->m_w, m_screen->m_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	//hide host cursor
	//SDL_ShowCursor(0);
	//renderer
	m_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	SDL_Texture *texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, m_screen->m_w, m_screen->m_h);
	//set blend mode
	SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

	//start up test task
	auto task = std::make_unique<GUI_Task>(m_router);
	auto task_id = task->start_task(task.get());

	//event loop
	while (m_running)
	{
		auto msg = mbox->poll();
		if (msg)
		{
			auto evt = (Event*)msg->begin();
			switch (evt->m_evt)
			{
			case evt_add_front:
			{
				//add view to screen
				auto body_struct = (Event_add_front*)evt;
				m_screen->add_front(body_struct->m_view);
				body_struct->m_view->dirty_all();
				auto reply = std::make_shared<Msg>();
				reply->set_dest(body_struct->m_reply);
				m_router.send(reply);
				break;
			}
			case evt_add_back:
			{
				//add view to screen
				auto body_struct = (Event_add_back*)evt;
				m_screen->add_back(body_struct->m_view);
				body_struct->m_view->dirty_all();
				auto reply = std::make_shared<Msg>();
				reply->set_dest(body_struct->m_reply);
				m_router.send(reply);
				break;
			}
			case evt_sub:
			{
				//sub view from screen
				auto body_struct = (Event_sub*)evt;
				body_struct->m_view->sub();
				m_screen->dirty_all();
				auto reply = std::make_shared<Msg>();
				reply->set_dest(body_struct->m_reply);
				m_router.send(reply);
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
			//resize back buffer and then do full redraw
			SDL_DestroyTexture(texture);
			SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, m_screen->m_w, m_screen->m_h);
			m_screen->set_flags(view_flag_dirty_all, view_flag_dirty_all);
			View::m_gui_flags |= view_flag_dirty_all;
			m_gui_flags = 0;
		}
		if (View::m_gui_flags)
		{
			SDL_SetRenderTarget(m_renderer, texture);
			composit();
			SDL_SetRenderTarget(m_renderer, 0);
			SDL_RenderCopy(m_renderer, texture, 0, 0);
			SDL_RenderPresent(m_renderer);
			View::m_gui_flags = 0;
		}

		//frame polling loop
		std::this_thread::sleep_for(std::chrono::milliseconds(GUI_FRAME_RATE));
	}

	//quit SDL
	SDL_DestroyWindow(sdl_window);
	SDL_Quit();

	//stop test task
	task->stop_thread();
	task->join_thread();

	//forget myself
	m_router.forget(entry);
}

void GUI_Service::composit()
{
	//iterate through views back to front
	//create visible region at root
	m_screen->backward_tree(
		[&](View &view)
		{
			//if not root
			auto parent = view.m_parent;
			if (parent)
			{
				//remove my opaque region from ancestors
				if ((view.m_flags & view_flag_opaque) != 0)
				{
					//remove entire view
					auto x = 0;
					auto y = 0;
					auto x1 = view.m_w;
					auto y1 = view.m_h;
					auto ancestor = &view;
					while ((parent = ancestor->m_parent))
					{
						//translate region
						auto px = ancestor->m_x;
						auto py = ancestor->m_y;
						auto px1 = parent->m_w;
						auto py1 = parent->m_h;
						x += px;
						y += py;
						x1 += px;
						y1 += py;
						//clip to parent, exit if clipped away
						if (x >= px1 || y >= py1 || x1 <= 0 || y1 <= 0) break;
						x = std::max(0, x);
						y = std::max(0, y);
						x1 = std::min(px1, x1);
						y1 = std::min(py1, y1);
						//remove opaque region
						parent->m_dirty.remove_rect(Rect(x, y, x1, y1));
						ancestor = parent;
					}
				}
				else
				{
					//remove opaque region
					Region vis_region;
					auto x = -view.m_x;
					auto y = -view.m_y;
					auto x1 = x + parent->m_w;
					auto y1 = y + parent->m_h;
					view.m_opaque.copy_rect(vis_region, Rect(x, y, x1, y1));
					auto ancestor = &view;
					while ((parent = ancestor->m_parent))
					{
						//exit if clipped away
						if (vis_region.m_region.empty()) break;
						//translate temp opaque region
						vis_region.translate(ancestor->m_x, ancestor->m_y);
						//clip temp opaque region
						vis_region.clip_rect(Rect(0, 0, parent->m_w, parent->m_h));
						//remove temp opaque region
						vis_region.remove_region(parent->m_dirty, 0, 0);
						ancestor = parent;
					}
				}
			}
			return true;
		},
		[&](View &view)
		{
			//clip local dirty region with parent bounds
			auto parent = view.m_parent;
			if (!parent) parent = &view;
			auto x = -view.m_x;
			auto y = -view.m_y;
			auto x1 = x + parent->m_w;
			auto y1 = y + parent->m_h;
			view.m_dirty.clip_rect(Rect(x, y, x1, y1));

			//paste local dirty region onto parent if not root
			parent = view.m_parent;
			if (parent)
			{
				x = view.m_x;
				y = view.m_y;
				view.m_dirty.paste_region(parent->m_dirty, x, y);
				//free local dirty region
				view.m_dirty.free();
			}

			//if dirty all flag then paste entire view onto parent
			if ((view.m_flags & view_flag_dirty_all) != 0)
			{
				//clear dirty all flag
				view.m_flags &= ~view_flag_dirty_all;
				x = view.m_x;
				y = view.m_y;
				x1 = x + view.m_w;
				y1 = y + view.m_h;
				parent = view.m_parent;
				if (!parent) parent = &view;
				parent->m_dirty.paste_rect(Rect(x, y, x1, y1));
			}
			return true;
		});

	//iterate through views front to back
	//distribute visible region
	//setting abs cords of views
	auto draw_list = std::forward_list<View*>{};
	view_pos abs;
	m_screen->forward_tree(
		[&](View &view)
		{
			//abs cords
			abs.m_x += view.m_x;
			abs.m_y += view.m_y;
			view.m_ctx.m_x = abs.m_x;
			view.m_ctx.m_y = abs.m_y;

			//copy view from parent if not root
			auto parent = view.m_parent;
			if (!parent) return true;

			//copy my area from parent
			auto x = view.m_ctx.m_x;
			auto y = view.m_ctx.m_y;
			auto x1 = x + view.m_w;
			auto y1 = y + view.m_h;
			parent->m_dirty.copy_rect(view.m_dirty, Rect(x, y, x1, y1));

			//do we have any dirty region ?
			if (view.m_dirty.m_region.empty()) return false;

			//remove my opaque region from ancestors
			if ((view.m_flags & view_flag_opaque) != 0)
			{
				//remove entire view
				auto ancestor = &view;
				while ((parent = ancestor->m_parent))
				{
					//clip to parent, exit if clipped away
					auto px = parent->m_ctx.m_x;
					auto py = parent->m_ctx.m_y;
					auto px1 = px + parent->m_w;
					auto py1 = py + parent->m_h;
					if (x >= px1 || y >= py1 || x1 <= 0 || y1 <= 0) break;
					x = std::max(px, x);
					y = std::max(py, y);
					x1 = std::min(px1, x1);
					y1 = std::min(py1, y1);
					parent->m_dirty.remove_rect(Rect(x, y, x1, y1));
					ancestor = parent;
				}
			}
			else
			{
				//remove opaque region
				auto ancestor = &view;
				while ((parent = ancestor->m_parent))
				{
					x = ancestor->m_ctx.m_x;
					y = ancestor->m_ctx.m_y;
					ancestor->m_opaque.remove_region(parent->m_dirty, x, y);
					ancestor = parent;
				}
			}

			//recursion if we have drawing
			return !view.m_dirty.m_region.empty();
		},
		[&](View &view)
		{
			//abs cords
			abs.m_x -= view.m_x;
			abs.m_y -= view.m_y;

			//add myself to draw list if not empty
			if (!view.m_dirty.m_region.empty())
			{
				view.m_ctx.m_view = &view;
				view.m_ctx.m_region = &view.m_dirty;
				view.m_ctx.m_renderer = m_renderer;
				draw_list.push_front(&view);
			}
			return true;
		});
	//draw all views on draw list, and free dirty regions.
	//at the end we have drawn all view areas that are dirty and visible.
	//plus we have freed all dirty regions and cleared all dirty_all flags.
	for (auto view : draw_list)
	{
		view->draw(view->m_ctx);
		view->m_dirty.free();
	}
}
