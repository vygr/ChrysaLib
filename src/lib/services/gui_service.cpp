#include "gui_service.h"
#include "../gui/region.h"
#include "../gui/ctx.h"
#include "../gui/backdrop.h"
#include "../gui/window.h"
#include "../gui/flow.h"
#include "../gui/grid.h"
#include "../gui/title.h"
#include "../gui/button.h"
#include "../gui/scroll.h"
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

	auto window = std::make_shared<Window>();
	auto window_flow = std::make_shared<Flow>();
	auto title_flow = std::make_shared<Flow>();
	auto button_grid = std::make_shared<Grid>();
	auto title = std::make_shared<Title>();
	auto min_button = std::make_shared<Button>();
	auto max_button = std::make_shared<Button>();
	auto close_button = std::make_shared<Button>();
	auto scroll = std::make_shared<Scroll>(scroll_flag_both);
	auto main_widget = std::make_shared<Button>();

	window_flow->def_prop("flow_flags", std::make_shared<Property>(flow_down_fill));
	title_flow->def_prop("flow_flags", std::make_shared<Property>(flow_left_fill));
	button_grid->def_prop("grid_height", std::make_shared<Property>(1));
	title->def_prop("text", std::make_shared<Property>("Some Test Text"));
	close_button->def_prop("text", std::make_shared<Property>("X"));
	min_button->def_prop("text", std::make_shared<Property>("-"));
	max_button->def_prop("text", std::make_shared<Property>("+"));
	scroll->def_prop("min_width", std::make_shared<Property>(128))
		->def_prop("min_height", std::make_shared<Property>(128));
	main_widget->def_prop("text", std::make_shared<Property>("main_widget"))
		->def_prop("min_width", std::make_shared<Property>(256))
		->def_prop("min_height", std::make_shared<Property>(256));

	window->add_child(window_flow);
	window_flow->add_child(title_flow)->add_child(scroll);
	title_flow->add_child(button_grid)->add_child(title);
	button_grid->add_child(min_button)->add_child(max_button)->add_child(close_button);
	scroll->add_child(main_widget);
	main_widget->change(0, 0, 256, 256);
	auto s = window->pref_size();
	window->change(107, 107, s.m_w, s.m_h);

	m_screen->add_back(window);

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

	//forget myself
	m_router.forget(entry);
}

void GUI_Service::composit()
{
	//iterate through views back to front, setting abs cords of views
	view_pos abs;
	m_screen->backward_tree(
		[&](View &view)
		{
			abs.m_x += view.m_x;
			abs.m_y += view.m_y;
			view.m_ctx.m_x = abs.m_x;
			view.m_ctx.m_y = abs.m_y;
			return true;
		},
		[&](View &view)
		{
			abs.m_x -= view.m_x;
			abs.m_y -= view.m_y;
			return true;
		});

	//iterate through views back to front
	//create visible region at root
	m_screen->backward_tree(
		[&](View &view)
		{
			//remove opaque region from ancestors if not root
			if (&view != &*m_screen)
			{
				//remove my opaque region from ancestors
				if ((view.m_flags & view_flag_opaque) != 0)
				{
					//remove entire view from ancestors
					auto x = 0;
					auto y = 0;
					auto x1 = view.m_w;
					auto y1 = view.m_h;
					auto view_o = &view;
					do
					{
						auto parent = view_o->m_parent;
						//translate region
						auto px = view_o->m_x;
						auto py = view_o->m_y;
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
						view_o = parent;
					} while (view_o != &*m_screen);
				}
				else
				{
					//temp visible region
					Region vis_region;
					//use opaque region, so my opaque area is the visible region
					auto parent = view.m_parent;
					auto x = -view.m_x;
					auto y = -view.m_y;
					auto x1 = x + parent->m_w;
					auto y1 = y + parent->m_h;
					view.m_opaque.copy_rect(vis_region, Rect(x, y, x1, y1));

					//remove from ancestors
					auto view_o = &view;
					do
					{
						parent = view.m_parent;
						//exit if clipped away
						if (vis_region.m_region.empty()) break;
						//translate temp opaque region
						vis_region.translate(view_o->m_x, view_o->m_y);
						//clip temp opaque region
						vis_region.clip_rect(Rect(0, 0, parent->m_w, parent->m_h));
						//remove temp opaque region
						vis_region.remove_region(parent->m_dirty, 0, 0);
						view_o = parent;
					} while (view_o != &*m_screen);

					//free any temp region
					vis_region.free();
				}
			}
			return true;
		},
		[&](View &view)
		{
			//clip local dirty region with parent bounds
			auto parent = view.m_parent;
			if (&view == &*m_screen) parent = &view;
			auto x = -view.m_x;
			auto y = -view.m_y;
			auto x1 = x + parent->m_w;
			auto y1 = y + parent->m_h;
			view.m_dirty.clip_rect(Rect(x, y, x1, y1));

			//paste local dirty region onto parent if not root
			if (&view != &*m_screen)
			{
				parent = view.m_parent;
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
				if (&view == &*m_screen) parent = &view;
				parent->m_dirty.paste_rect(Rect(x, y, x1, y1));
			}
			return true;
		});

	//iterate through views front to back
	//distribute visible region
	auto draw_list = std::forward_list<View*>{};
	m_screen->forward_tree(
		[&](View &view)
		{
			//copy view from parent if not root
			if (&view == &*m_screen) return true;

			//remove opaque region from ancestors
			auto parent = view.m_parent;
			auto x = view.m_ctx.m_x;
			auto y = view.m_ctx.m_y;
			auto x1 = x + view.m_w;
			auto y1 = y + view.m_h;
			//copy my area from parent
			parent->m_dirty.copy_rect(view.m_dirty, Rect(x, y, x1, y1));

			//did we find any opaque region ?
			if (view.m_dirty.m_region.empty()) return false;

			//remove my opaque region from ancestors
			if ((view.m_flags & view_flag_opaque) != 0)
			{
				//remove entire view from ancestors
				auto view_o = &view;
				do
				{
					parent = view_o->m_parent;
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
					//remove opaque region
					parent->m_dirty.remove_rect(Rect(x, y, x1, y1));
					view_o = parent;
				} while (view_o != &*m_screen);
			}
			else
			{
				//remove opaque region from ancestors
				auto view_o = &view;
				do
				{
					parent = view_o->m_parent;
					x = view.m_ctx.m_x;
					y = view.m_ctx.m_y;
					view.m_opaque.remove_region(parent->m_dirty, x, y);
					view_o = parent;
				} while (view_o != &*m_screen);
			}

			//recursion if we have drawing
			return !view.m_dirty.m_region.empty();
		},
		[&](View &view)
		{
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
	//draw all views on draw list, and free dirty regions
	for (auto view : draw_list)
	{
		view->draw(view->m_ctx);
		view->m_dirty.free();
	}
}
