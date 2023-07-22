#include "gui_service.h"
#include "kernel_service.h"
#include "../gui/region.h"
#include "../gui/ctx.h"
#include "../gui/backdrop.h"
#include "../gui/colors.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <string.h>

#include "../apps/launcher/app.h"

#if _HOST_GUI == 1
extern "C" void host_gui_init(SDL_Rect *rect, uint64_t flags);
extern "C" void host_gui_deinit();
extern "C" uint64_t host_gui_poll_event(void *event);
extern "C" void host_gui_begin_composite();
extern "C" void host_gui_end_composite();
extern "C" void host_gui_flush(const SDL_Rect *rect);
extern "C" void host_gui_resize(uint64_t w, uint64_t h);
#else
extern void host_gui_init(SDL_Rect *rect, uint64_t flags);
extern void host_gui_deinit();
extern uint64_t host_gui_poll_event(void *event);
extern void host_gui_begin_composite();
extern void host_gui_end_composite();
extern void host_gui_flush(const SDL_Rect *rect);
extern void host_gui_resize(uint64_t w, uint64_t h);
#endif

//////
// gui
//////

void GUI_Service::run()
{
	//get my mailbox address, id was allocated in the constructor
	auto mbox = global_router->validate(m_net_id);
	auto entry = global_router->declare(m_net_id, "gui", "GUI_Service v0.1");

	m_screen = std::make_shared<Backdrop>();
	m_screen->change(0, 0, 1280, 960)->dirty_all()->def_props({
		{"color", argb_grey2},
		{"ink_color", argb_grey1},
		});

	//init SDL
	Kernel_Service::callback([&]()
	{
		SDL_Rect r = {0, 0, m_screen->m_w, m_screen->m_h};
		host_gui_init(&r, 0);
		m_screen->m_w = r.w;
		m_screen->m_h = r.h;
	});

	//start up launcher task
	auto app = std::make_shared<Launcher_App>();
	std::shared_ptr<Msg> msg;
	Kernel_Service::start_task(app);
	while ((msg = mbox->poll()) == nullptr)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(GUI_FRAME_RATE));
	}
	goto body;

	//event loop
	while (m_running)
	{
		while ((msg = mbox->poll()))
		{
		body:
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
				event_body->m_view->hide();
				event_body->m_view->sub();
				auto reply = std::make_shared<Msg>();
				reply->set_dest(event_body->m_reply);
				global_router->send(reply);
				break;
			}
			case evt_locate:
			{
				//locate view on screen
				auto event_body = (Event_locate*)body;
				auto w = event_body->m_w;
				auto h = event_body->m_h;
				auto pos = event_body->m_pos;
				auto reply = std::make_shared<Msg>(sizeof(locate_reply));
				auto reply_body = (locate_reply*)reply->begin();
				reply->set_dest(event_body->m_reply);
				//fit to screen
				auto sw = m_screen->m_w;
				auto sh = m_screen->m_h;
				auto mx = m_mouse_x;
				auto my = m_mouse_y;
				auto x = mx - w / 2;
				auto y = my - h / 2;
				switch (pos)
				{
				case GUI_Task::locate_top:
					y = my;
					break;
				case GUI_Task::locate_left:
					x = mx;
					break;
				case GUI_Task::locate_bottom:
					y = my - h + 1;
					break;
				case GUI_Task::locate_right:
					x = mx - w + 1;
					break;
				default:
					break;
				}
				x = std::max(0, std::min(x, sw - w));
				y = std::max(0, std::min(y, sh - h));
				w = std::min(w, sw);
				h = std::min(h, sh);
				reply_body->m_bounds.m_x = x;
				reply_body->m_bounds.m_y = y;
				reply_body->m_bounds.m_w = w;
				reply_body->m_bounds.m_h = h;
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
			while (host_gui_poll_event(&e))
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
				host_gui_resize(m_screen->m_w, m_screen->m_h);
				m_screen->dirty_all();
				View::m_gui_flags &= ~view_flag_screen;
			}
			if ((View::m_gui_flags & view_flag_dirty_all) != 0)
			{
				host_gui_begin_composite();
				composite();
				host_gui_end_composite();
				SDL_Rect r = {0, 0, m_screen->m_w, m_screen->m_h};
				host_gui_flush(&r);
				View::m_gui_flags &= ~view_flag_dirty_all;
			}

			//silent removal of temp views
			for (auto &view : View::m_temps) view->sub();
			View::m_temps.clear();
		});

		//frame polling loop, then exit if no children
		std::this_thread::sleep_for(std::chrono::milliseconds(GUI_FRAME_RATE));
		if (m_screen->m_children.empty()) break;
	}

	Kernel_Service::callback([&]()
	{
		host_gui_deinit();
	});

	//forget myself
	global_router->forget(entry);

	//stop myself !!!
	Kernel_Service::stop_task(shared_from_this());
	Kernel_Service::join_task(shared_from_this());
	//ask kernel to exit !!!
	Kernel_Service::exit();
}

void GUI_Service::composite()
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

GUI_Service *GUI_Service::quit(const SDL_Event &e)
{
	//send close to all children
	auto children = m_screen->children();
	for (auto &child : children)
	{
		auto owner = child->find_owner();
		if (owner != Net_ID())
		{
			auto msg = std::make_shared<Msg>(sizeof(View::Event));
			auto event_body = (View::Event*)msg->begin();
			msg->set_dest(owner);
			event_body->m_evt = evt_exit;
			event_body->m_type = ev_type_gui;
			global_router->send(msg);
		}
	}
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
				event_body->m_evt = m_mouse_id;
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
			event_body->m_evt = m_mouse_id;
			global_router->send(msg);
		}
	}
	return view;
}

GUI_Service *GUI_Service::mouse_wheel(const SDL_MouseWheelEvent &e)
{
	auto view = set_mouse_id();
	auto owner = view->find_owner();
	if (owner != Net_ID())
	{
		auto msg = std::make_shared<Msg>(sizeof(View::Event_wheel));
		auto event_body = (View::Event_wheel*)msg->begin();
		msg->set_dest(owner);
		event_body->m_type = ev_type_wheel;
		event_body->m_evt = m_mouse_id;
		event_body->m_x = e.x;
		event_body->m_y = e.y;
		event_body->m_direction = e.direction;
		global_router->send(msg);
	}
	return this;
}

GUI_Service *GUI_Service::mouse_button_down(const SDL_MouseButtonEvent &e)
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
		event_body->m_evt = m_mouse_id;
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

GUI_Service *GUI_Service::mouse_button_up(const SDL_MouseButtonEvent &e)
{
	m_mouse_x = e.x;
	m_mouse_y = e.y;
	m_mouse_buttons &= ~e.button;
	if (auto view = m_screen->find_id(m_mouse_id))
	{
		auto owner = view->find_owner();
		if (owner != Net_ID())
		{
			auto msg = std::make_shared<Msg>(sizeof(View::Event_mouse));
			auto event_body = (View::Event_mouse*)msg->begin();
			msg->set_dest(owner);
			event_body->m_type = ev_type_mouse;
			event_body->m_evt = m_mouse_id;
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

GUI_Service *GUI_Service::mouse_motion(const SDL_MouseMotionEvent &e)
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
			event_body->m_evt = m_mouse_id;
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
		auto keys = "§1234567890-=qwertyuiop[]asdfghjkl;\\`zxcvbnm,./'";
		auto cooked_keys = "±!@#$%^&*()_+QWERTYUIOP{}ASDFGHJKL:|~ZXCVBNM<>?\"";
		auto itr = strchr(keys, key_code);
		if (itr) key = cooked_keys[itr - keys];
	}
	return key;
}

GUI_Service *GUI_Service::key_down(const SDL_KeyboardEvent &e)
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
		event_body->m_evt = m_mouse_id;
		event_body->m_keycode = key_code;
		event_body->m_key = cook_key(key_code, key, mod);
		event_body->m_mod = mod;
		global_router->send(msg);
	}
	return this;
}

GUI_Service *GUI_Service::window_event(const SDL_WindowEvent &e)
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
				event_body->m_evt = child->get_id();
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
