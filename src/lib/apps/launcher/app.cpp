#include "app.h"
#include "../../gui/ui.h"

#include "../canvas/app.h"
#include "../services/app.h"
#include "../mandelbrot/app.h"
#include "../raymarch/app.h"

std::string to_utf8(uint32_t c);
uint32_t from_utf8(uint8_t **data);

void Launcher_App::run()
{
	enum
	{
		event_close, //must be first !
		event_button,
	};

	ui_window(window, ({}))
		ui_flow(window_flow, ({
			{"flow_flags", flow_down_fill}}))
			ui_flow(title_flow, ({
				{"flow_flags", flow_left_fill}}))
				ui_grid(button_grid, ({
					{"grid_height", 1},
					{"font", Font::open("fonts/Entypo.ctf", 22)}}))
					ui_button(close_button, ({
						{"text", to_utf8(0xea19)}}))
					ui_end
				ui_end
				ui_title(title, ({
					{"text", "Launcher"}}))
				ui_end
			ui_end
			ui_grid(app_grid, ({
				{"color" , Property::get_default("env_toolbar2_col")->get_long()},
				{"grid_width", 2}}))
				ui_button(_1, ({
					{"text", "services"}}))
					ui_connect(event_button)
				ui_end
				ui_button(_2, ({
					{"text", "canvas"}}))
					ui_connect(event_button)
				ui_end
				ui_button(_3, ({
					{"text", "mandelbrot"}}))
					ui_connect(event_button)
				ui_end
				ui_button(_4, ({
					{"text", "raymarch"}}))
					ui_connect(event_button)
				ui_end
			ui_end
		ui_end
	ui_end

	//add to my GUI screen
	auto s = window->pref_size();
	window->change(0, 0, s.m_w * 110 / 100, s.m_h);
	add_front(window);

	//event loop
	auto mbox = global_router->validate(get_id());
	while (m_running)
	{
		auto msg = mbox->read();
		auto body = (View::Event*)msg->begin();
		switch (body->m_evt)
		{
		case event_button:
		{
			//launch app
			auto action_body = (View::Event_action*)body;
			auto source_id = action_body->m_source_id;
			auto source = window->find_id(source_id);
			auto text = source->get_string_prop("text");
			if (text == "services")
			{
				Kernel_Service::start_task(Services_App::create());
			}
			else if (text == "canvas")
			{
				Kernel_Service::start_task(Canvas_App::create());
			}
			else if (text == "mandelbrot")
			{
				Kernel_Service::start_task(Mandelbrot_App::create());
			}
			else if (text == "raymarch")
			{
				Kernel_Service::start_task(Raymarch_App::create());
			}
			break;
		}
		case event_close:
		{
			Kernel_Service::stop_task(shared_from_this());
			break;
		}
		default:
		{
			//dispatch to widgets
			window->event(msg);
			break;
		}
		}
	}

	//tidy up
	sub(window);
}
