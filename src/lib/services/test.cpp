#include "test.h"
#include "../gui/colors.h"
#include "../gui/backdrop.h"
#include "../gui/window.h"
#include "../gui/flow.h"
#include "../gui/grid.h"
#include "../gui/title.h"
#include "../gui/button.h"
#include "../gui/scroll.h"
#include "../gui/canvas.h"
#include "../gui/path.h"

////////////
// test task
////////////

void Test_Task::run()
{
	//get my mailbox address, id was allocated in the constructor
	auto mbox = global_router->validate(m_net_id);

	//test UI app
	auto window = std::make_shared<Window>();
	auto window_flow = std::make_shared<Flow>();
	auto title_flow = std::make_shared<Flow>();
	auto button_grid = std::make_shared<Grid>();
	auto title = std::make_shared<Title>();
	auto min_button = std::make_shared<Button>();
	auto max_button = std::make_shared<Button>();
	auto close_button = std::make_shared<Button>();
	auto scroll = std::make_shared<Scroll>(scroll_flag_both);
	auto main_widget = std::make_shared<Canvas>(256, 256, 1);

	window_flow->def_prop("flow_flags", std::make_shared<Property>(flow_down_fill));
	title_flow->def_prop("flow_flags", std::make_shared<Property>(flow_left_fill));
	button_grid->def_prop("grid_height", std::make_shared<Property>(1));
	title->def_prop("text", std::make_shared<Property>("Some Test Text"));
	close_button->def_prop("text", std::make_shared<Property>("X"));
	min_button->def_prop("text", std::make_shared<Property>("-"));
	max_button->def_prop("text", std::make_shared<Property>("+"));
	scroll->def_prop("min_width", std::make_shared<Property>(256))
		->def_prop("min_height", std::make_shared<Property>(256));

	window->add_child(window_flow);
	window_flow->add_child(title_flow)->add_child(scroll);
	title_flow->add_child(button_grid)->add_child(title);
	button_grid->add_child(min_button)->add_child(max_button)->add_child(close_button);
	scroll->add_child(main_widget);
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
	main_widget->fpoly(polygon, 0, 0, winding_odd_even);
	main_widget->swap();

	//add to my GUI screen
	add_front(window);

	//event loop
	while (m_running)
	{
		auto msg = mbox->read();
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
