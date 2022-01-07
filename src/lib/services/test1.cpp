#include "test1.h"
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

std::string to_utf8(uint32_t c);
uint32_t from_utf8(uint8_t **data);

////////////
// test task
////////////

void Test_1::run()
{
	//get my mailbox address, id was allocated in the constructor
	auto mbox = global_router->validate(m_net_id);

	auto ui_root = std::vector<std::shared_ptr<View>>{};

		auto window = std::make_shared<Window>();
		ui_root.push_back(window);

			auto window_flow = std::make_shared<Flow>();
			ui_root.push_back(window_flow);
			ui_root[ui_root.size() - 2]->add_child(ui_root.back());
			ui_root.back()->def_props({
				{"flow_flags", flow_down_fill}
				});

				auto title_flow = std::make_shared<Flow>();
				ui_root.push_back(title_flow);
				ui_root[ui_root.size() - 2]->add_child(ui_root.back());
				ui_root.back()->def_props({
					{"flow_flags", flow_left_fill}
					});

					auto button_grid = std::make_shared<Grid>();
					ui_root.push_back(button_grid);
					ui_root[ui_root.size() - 2]->add_child(ui_root.back());
					ui_root.back()->def_props({
						{"grid_height", 1},
						{"font", Font::open("fonts/Entypo.ctf", 22)},
						});

						auto min_button = std::make_shared<Button>();
						ui_root.push_back(min_button);
						ui_root[ui_root.size() - 2]->add_child(ui_root.back());
						ui_root.back()->def_props({
							{"text", to_utf8(0xea1a)},
							});
						ui_root.pop_back();

						auto max_button = std::make_shared<Button>();
						ui_root.push_back(max_button);
						ui_root[ui_root.size() - 2]->add_child(ui_root.back());
						ui_root.back()->def_props({
							{"text", to_utf8(0xea1b)},
							});
						ui_root.pop_back();

						auto close_button = std::make_shared<Button>();
						ui_root.push_back(close_button);
						ui_root[ui_root.size() - 2]->add_child(ui_root.back());
						ui_root.back()->def_props({
							{"text", to_utf8(0xea19)},
							});
						ui_root.pop_back();
					ui_root.pop_back();

					auto title = std::make_shared<Title>();
					ui_root.push_back(title);
					ui_root[ui_root.size() - 2]->add_child(ui_root.back());
					ui_root.back()->def_props({
						{"text", "Some Test Text"},
						});
					ui_root.pop_back();
				ui_root.pop_back();

				auto scroll = std::make_shared<Scroll>(scroll_flag_both);
				ui_root.push_back(scroll);
				ui_root[ui_root.size() - 2]->add_child(ui_root.back());
				ui_root.back()->def_props({
					{"min_width", 256},
					{"min_height", 256},
					});

					auto main_widget = std::make_shared<Canvas>(256, 256, 1);
					ui_root.push_back(main_widget);
					ui_root[ui_root.size() - 2]->add_child(ui_root.back());
					ui_root.back()->def_props({
						{"flow_flags", flow_right_fill},
						});
					ui_root.pop_back();
				ui_root.pop_back();
			ui_root.pop_back();
		ui_root.pop_back();

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
