#include "test1.h"
#include "../gui/ui.h"
#include <assert.h>

std::string to_utf8(uint32_t c);
uint32_t from_utf8(uint8_t **data);

////////////
// test task
////////////

void Test_1::run()
{
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
					ui_end
				ui_end
				ui_title(title, ({
					{"text", "Some Test Text"}}))
				ui_end
			ui_end
			ui_scroll(scroll, scroll_flag_both, ({
				{"min_width", 256},
				{"min_height", 256}}))
				ui_canvas(main_widget, 256, 256, 1, ({}))
				ui_end
			ui_end
		ui_end
	ui_end

	auto s = window->pref_size();
	window->change(107, 107, s.m_w, s.m_h);

	//draw a polygon on the canvas !!!
	auto path = Path();
	path.gen_cubic(
		10 << FP_SHIFT, 10 << FP_SHIFT,
		250 << FP_SHIFT, 10 << FP_SHIFT,
		10 << FP_SHIFT, 250 << FP_SHIFT,
		250 << FP_SHIFT, 250 << FP_SHIFT, 1 << FP_SHIFT);
	path.gen_cubic(
		250 << FP_SHIFT, 250 << FP_SHIFT,
		250 << FP_SHIFT, 10 << FP_SHIFT,
		10 << FP_SHIFT, 250 << FP_SHIFT,
		10 << FP_SHIFT, 10 << FP_SHIFT, 1 << FP_SHIFT);
	auto polygon = std::vector<Path>{path};
	main_widget->set_canvas_flags(canvas_flag_antialias);
	main_widget->set_col(argb_green);
	main_widget->fbox(5, 5, 240, 240);
	main_widget->set_col(argb_black);
	main_widget->fpoly(polygon, 0 << FP_SHIFT, 0 << FP_SHIFT, winding_odd_even);
	main_widget->swap();

	//add to my GUI screen
	add_front(window);

	//event loop
	const auto select = std::vector<Net_ID>{m_net_id};
	while (m_running)
	{
		auto idx = global_router->select(select);
		assert(idx == 0);
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
}
