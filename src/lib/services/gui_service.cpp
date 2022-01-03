#include "gui_service.h"
#include "kernel_service.h"
#include "test.h"
#include "../gui/region.h"
#include "../gui/ctx.h"
#include "../gui/backdrop.h"
#include "../gui/colors.h"
#include <iostream>
#include <sstream>

//////
// gui
//////

void GUI_Service::run()
{
	//get my mailbox address, id was allocated in the constructor
	auto mbox = global_router->validate(m_net_id);
	auto entry = global_router->declare(m_net_id, "gui", "GUI_Service v0.1");

	m_screen = std::make_shared<Backdrop>();
	m_screen->def_prop("color", std::make_shared<Property>(argb_grey2))
		->def_prop("ink_color", std::make_shared<Property>(argb_grey1))
		->change(0, 0, 1280, 960)->dirty_all();

	//init SDL
	Kernel_Service::callback([&]()
	{
		SDL_SetMainReady();
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
		//create window
		m_sdl_window = SDL_CreateWindow("GUI Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_screen->m_w, m_screen->m_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		//hide host cursor
		//SDL_ShowCursor(0);
		//renderer
		m_renderer = SDL_CreateRenderer(m_sdl_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
		m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, m_screen->m_w, m_screen->m_h);
		//set blend mode
		SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
	});

	//start up test tasks
	auto task1 = std::make_unique<Test_Task>();
	auto task1_id = Kernel_Service::start_task(task1.get());
	auto task2 = std::make_unique<Test_Task>();
	auto task2_id = Kernel_Service::start_task(task2.get());

	//event loop
	while (m_running)
	{
		while (auto msg = mbox->poll())
		{
			auto body = (Event*)msg->begin();
			switch (body->m_evt)
			{
			case evt_add_front:
			{
				//add view to screen
				auto event_body = (Event_add_front*)body;
				m_screen->add_front(event_body->m_view);
				event_body->m_view->dirty_all();
				auto reply = std::make_shared<Msg>();
				reply->set_dest(event_body->m_reply);
				global_router->send(reply);
				break;
			}
			case evt_add_back:
			{
				//add view to screen
				auto event_body = (Event_add_back*)body;
				m_screen->add_back(event_body->m_view);
				event_body->m_view->dirty_all();
				auto reply = std::make_shared<Msg>();
				reply->set_dest(event_body->m_reply);
				global_router->send(reply);
				break;
			}
			case evt_sub:
			{
				//sub view from screen
				auto event_body = (Event_sub*)body;
				event_body->m_view->sub();
				m_screen->dirty_all();
				auto reply = std::make_shared<Msg>();
				reply->set_dest(event_body->m_reply);
				global_router->send(reply);
				break;
			}
			default:
				break;
			}
		}

		Kernel_Service::callback([&]()
		{
			std::lock_guard<std::recursive_mutex> lock(View::m_mutex);
			SDL_Event e;
			while (SDL_PollEvent(&e))
			{
				if (e.type == SDL_QUIT) quit(e);
				else if (e.type == SDL_KEYDOWN) key_down((SDL_KeyboardEvent&)e);
				else if (e.type == SDL_MOUSEWHEEL) mouse_wheel((SDL_MouseWheelEvent&)e);
				else if (e.type == SDL_MOUSEBUTTONDOWN) mouse_button_down((SDL_MouseButtonEvent&)e);
				else if (e.type == SDL_MOUSEBUTTONUP) mouse_button_up((SDL_MouseButtonEvent&)e);
				else if (e.type == SDL_MOUSEMOTION) mouse_motion((SDL_MouseMotionEvent&)e);
				else if (e.type == SDL_WINDOWEVENT) window_event((SDL_WindowEvent&)e);
			}

			if ((View::m_gui_flags & view_flag_screen) != 0)
			{
				//resize back buffer and then do full redraw
				SDL_DestroyTexture(m_texture);
				m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, m_screen->m_w, m_screen->m_h);
				m_screen->dirty_all();
				View::m_gui_flags &= ~view_flag_screen;
			}
			if ((View::m_gui_flags & view_flag_dirty_all) != 0)
			{
				SDL_SetRenderTarget(m_renderer, m_texture);
				composit();
				SDL_SetRenderTarget(m_renderer, 0);
				SDL_RenderCopy(m_renderer, m_texture, 0, 0);
				SDL_RenderPresent(m_renderer);
				View::m_gui_flags &= ~view_flag_dirty_all;
			}

			//silent removal of temp views
			for (auto &view : View::m_temps) view->sub();
			View::m_temps.clear();
		});

		//frame polling loop
		std::this_thread::sleep_for(std::chrono::milliseconds(GUI_FRAME_RATE));
	}

	Kernel_Service::callback([&]()
	{
		//quit SDL
		SDL_DestroyWindow(m_sdl_window);
		SDL_Quit();
	});

	//stop test task
	task1->stop_thread();
	task1->join_thread();
	task2->stop_thread();
	task2->join_thread();

	//forget myself
	global_router->forget(entry);

	//ask kernel to exit !!!
	Kernel_Service::exit();
}

void GUI_Service::composit()
{
	//iterate through views back to front
	//create visible region at root
	//setting abs cords of views
	view_pos abs;
	m_screen->backward_tree(
		[&](View &view)
		{
			//abs cords
			abs.m_x += view.m_x;
			abs.m_y += view.m_y;
			view.m_ctx.m_x = abs.m_x;
			view.m_ctx.m_y = abs.m_y;

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
			//abs cords
			abs.m_x -= view.m_x;
			abs.m_y -= view.m_y;

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
	auto draw_list = std::forward_list<View*>{};
	m_screen->forward_tree(
		[&](View &view)
		{
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

GUI_Service *GUI_Service::quit(SDL_Event &e)
{
	m_running = false;
	return this;
}

View *GUI_Service::set_mouse_id()
{
	auto view = m_screen->hit_tree(m_mouse_x, m_mouse_y);
	auto mouse_id = view->get_id();
	if (mouse_id != m_mouse_id)
	{
		if (auto old_view = m_screen->find_id(m_mouse_id))
		{
			auto owner = old_view->find_owner();
			if (owner != Net_ID())
			{
				auto msg = std::make_shared<Msg>(sizeof(View::Event_exit));
				auto event_body = (View::Event_exit*)msg->begin();
				msg->set_dest(owner);
				event_body->m_type = ev_type_exit;
				event_body->m_target_id = m_mouse_id;
				global_router->send(msg);
			}
		}
		m_mouse_id = mouse_id;
		auto owner = view->find_owner();
		if (owner != Net_ID())
		{
			auto msg = std::make_shared<Msg>(sizeof(View::Event_enter));
			auto event_body = (View::Event_enter*)msg->begin();
			msg->set_dest(owner);
			event_body->m_type = ev_type_enter;
			event_body->m_target_id = m_mouse_id;
			global_router->send(msg);
		}
	}
	return view;
}

GUI_Service *GUI_Service::mouse_wheel(SDL_MouseWheelEvent &e)
{
	auto view = set_mouse_id();
	auto owner = view->find_owner();
	if (owner != Net_ID())
	{
		auto msg = std::make_shared<Msg>(sizeof(View::Event_wheel));
		auto event_body = (View::Event_wheel*)msg->begin();
		msg->set_dest(owner);
		event_body->m_type = ev_type_wheel;
		event_body->m_target_id = m_mouse_id;
		event_body->m_x = e.x;
		event_body->m_y = e.y;
		event_body->m_direction = e.direction;
		global_router->send(msg);
	}
	return this;
}

GUI_Service *GUI_Service::mouse_button_down(SDL_MouseButtonEvent &e)
{
	m_mouse_x = e.x;
	m_mouse_y = e.y;
	m_mouse_buttons |= e.button;
	auto view = set_mouse_id();
	auto owner = view->find_owner();
	if (owner != Net_ID())
	{
		auto msg = std::make_shared<Msg>(sizeof(View::Event_mouse));
		auto event_body = (View::Event_mouse*)msg->begin();
		msg->set_dest(owner);
		event_body->m_type = ev_type_mouse;
		event_body->m_target_id = m_mouse_id;
		event_body->m_x = m_mouse_x;
		event_body->m_y = m_mouse_y;
		event_body->m_rx = m_mouse_x - view->m_ctx.m_x;
		event_body->m_ry = m_mouse_y - view->m_ctx.m_y;
		event_body->m_buttons = m_mouse_buttons;
		event_body->m_count = e.clicks;
		global_router->send(msg);
	}
	return this;
}

GUI_Service *GUI_Service::mouse_button_up(SDL_MouseButtonEvent &e)
{
	m_mouse_x = e.x;
	m_mouse_y = e.y;
	m_mouse_buttons ^= e.button;
	if (auto view = m_screen->find_id(m_mouse_id))
	{
		auto owner = view->find_owner();
		if (owner != Net_ID())
		{
			auto msg = std::make_shared<Msg>(sizeof(View::Event_mouse));
			auto event_body = (View::Event_mouse*)msg->begin();
			msg->set_dest(owner);
			event_body->m_type = ev_type_mouse;
			event_body->m_target_id = m_mouse_id;
			event_body->m_x = m_mouse_x;
			event_body->m_y = m_mouse_y;
			event_body->m_rx = m_mouse_x - view->m_ctx.m_x;
			event_body->m_ry = m_mouse_y - view->m_ctx.m_y;
			event_body->m_buttons = m_mouse_buttons;
			event_body->m_count = e.clicks;
			global_router->send(msg);
		}
	}
	return this;
}

GUI_Service *GUI_Service::mouse_motion(SDL_MouseMotionEvent &e)
{
	m_mouse_x = e.x;
	m_mouse_y = e.y;
	m_mouse_buttons = e.state;
	if (!m_mouse_buttons) set_mouse_id();
	if (auto view = m_screen->find_id(m_mouse_id))
	{
		auto owner = view->find_owner();
		if (owner != Net_ID())
		{
			auto msg = std::make_shared<Msg>(sizeof(View::Event_mouse));
			auto event_body = (View::Event_mouse*)msg->begin();
			msg->set_dest(owner);
			event_body->m_type = ev_type_mouse;
			event_body->m_target_id = m_mouse_id;
			event_body->m_x = m_mouse_x;
			event_body->m_y = m_mouse_y;
			event_body->m_rx = m_mouse_x - view->m_ctx.m_x;
			event_body->m_ry = m_mouse_y - view->m_ctx.m_y;
			event_body->m_buttons = m_mouse_buttons;
			event_body->m_count = 0;
			global_router->send(msg);
		}
	}
	return this;
}

int cook_key(uint32_t key_code, uint32_t key, uint32_t mod)
{
	if (mod & (ev_key_mod_caps_lock | ev_key_mod_shift))
	{
		auto idx = 0;
		auto keys = "§1234567890-=qwertyuiop[]asdfghjkl;\\`zxcvbnm,./'";
		auto cooked_keys = "±!@#$%^&*()_+QWERTYUIOP{}ASDFGHJKL:|~ZXCVBNM<>?\"";
		auto itr = strchr(keys, key_code);
		if (itr) key = cooked_keys[itr - keys];
	}
	return key;
}

GUI_Service *GUI_Service::key_down(SDL_KeyboardEvent &e)
{
	auto view = set_mouse_id();
	auto key_code = e.keysym.scancode;
	auto key = e.keysym.sym;
	auto mod = e.keysym.mod;
	auto owner = view->find_owner();
	if (owner != Net_ID())
	{
		auto msg = std::make_shared<Msg>(sizeof(View::Event_key));
		auto event_body = (View::Event_key*)msg->begin();
		msg->set_dest(owner);
		event_body->m_type = ev_type_key;
		event_body->m_target_id = m_mouse_id;
		event_body->m_keycode = key_code;
		event_body->m_key = cook_key(key_code, key, mod);
		event_body->m_mod = mod;
		global_router->send(msg);
	}
	return this;
}

GUI_Service *GUI_Service::window_event(SDL_WindowEvent &e)
{
	auto event = e.event;
	if (event == SDL_WINDOWEVENT_SIZE_CHANGED)
	{
		m_screen->set_bounds(0, 0, e.data1, e.data2);
		auto children = m_screen->children();
		for (auto &child : children)
		{
			auto owner = child->find_owner();
			if (owner != Net_ID())
			{
				auto msg = std::make_shared<Msg>(sizeof(View::Event_gui));
				auto event_body = (View::Event_gui*)msg->begin();
				msg->set_dest(owner);
				event_body->m_type = ev_type_gui;
				event_body->m_target_id = child->get_id();
				global_router->send(msg);
			}
		}
		m_screen->set_flags(view_flag_dirty_all | view_flag_screen, view_flag_dirty_all | view_flag_screen);
	}
	else if (event == SDL_WINDOWEVENT_SHOWN
		|| event == SDL_WINDOWEVENT_RESTORED)
	{
		m_screen->set_flags(view_flag_dirty_all, view_flag_dirty_all);
	}
	return this;
}
