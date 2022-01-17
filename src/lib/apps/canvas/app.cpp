#include "app.h"
#include "../../gui/ui.h"

std::string to_utf8(uint32_t c);
uint32_t from_utf8(uint8_t **data);

void Canvas_App::run()
{
	enum
	{
		select_main,
		select_size,
	};

	enum
	{
		event_close, //must be first !
	};

	ui_window(window, ({}))
		ui_flow(window_flow, ({
			{"flow_flags", flow_down_fill}}))
			ui_flow(title_flow, ({
				{"flow_flags", flow_left_fill}}))
				ui_grid(button_grid, ({
					{"grid_height", 1},
					{"font", Font::open("fonts/Entypo.ctf", 22)}}))
					ui_button(min_button, ({
						{"text", to_utf8(0xea1a)}}))
					ui_end
					ui_button(max_button, ({
						{"text", to_utf8(0xea1b)}}))
					ui_end
					ui_button(close_button, ({
						{"text", to_utf8(0xea19)}}))
						ui_connect(event_close)
					ui_end
				ui_end
				ui_title(title, ({
					{"text", "Canvas"}}))
				ui_end
			ui_end
			ui_scroll(scroll, scroll_flag_both, ({
				{"min_width", 256},
				{"min_height", 256}}))
				ui_canvas(canvas, 256, 256, 1, ({}))
				ui_end
			ui_end
		ui_end
	ui_end

	//draw a polygon on the canvas !!!
	auto path = Path();
	path.gen_cubic(
		10.0, 10.0,
		250.0, 10.0,
		10.0, 250.0,
		250.0, 250.0, 0.25);
	path.gen_cubic(
		250.0, 250.0,
		250.0, 10.0,
		10.0, 250.0,
		10.0, 10.0, 0.25);
	auto polygon = std::vector<Path>{path};
	canvas->set_canvas_flags(canvas_flag_antialias);
	canvas->set_col(argb_green);
	canvas->fbox(5, 5, 240, 240);
	canvas->set_col(argb_black);
	canvas->fpoly(polygon, 0.0, 0.0, winding_odd_even);
	canvas->swap();

	//add to my GUI screen
	window->change(locate(window->pref_size()));
	add_front(window);

	//event loop
	auto select = alloc_select(select_size);
	while (m_running)
	{
		auto idx = global_router->select(select);
		auto msg = global_router->read(select[idx]);
		auto body = (View::Event*)msg->begin();
		switch (body->m_evt)
		{
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
	free_select(select);
}
