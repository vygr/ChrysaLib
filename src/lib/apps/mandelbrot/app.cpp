#include "app.h"
#include "../../gui/ui.h"
#include <assert.h>

std::string to_utf8(uint32_t c);
uint32_t from_utf8(uint8_t **data);

void Mandelbrot_App::run()
{
	enum
	{
		select_main,
		select_size,
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
					{"text", "Mandelbrot"}}))
				ui_end
			ui_end
			ui_canvas(canvas, 512, 512, 1, ({}))
			ui_end
		ui_end
	ui_end

	auto s = window->pref_size();
	window->change(120, 64, s.m_w, s.m_h);

	//clear canvas
	canvas->set_col(argb_black);
	canvas->fill();
	canvas->swap();

	//add to my GUI screen
	add_front(window);

	//event loop
	auto select = alloc_select(select_size);
	while (m_running)
	{
		auto idx = global_router->select(select);
		auto msg = global_router->read(select[idx]);
		auto body = (Event*)msg->begin();
		switch (body->m_evt)
		{
		default:
		{
			//dispatch to widgets
			window->event(msg);
			break;
		}
		}
	}
	free_select(select);
}
