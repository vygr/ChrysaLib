#include "test2.h"
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

void Test_2::run()
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
						{"text", "Services"},
						});
					ui_root.pop_back();
				ui_root.pop_back();

				auto main_widget = std::make_shared<Flow>();
				ui_root.push_back(main_widget);
				ui_root[ui_root.size() - 2]->add_child(ui_root.back());
				ui_root.back()->def_props({
					{"flow_flags", flow_right_fill},
					});

					auto flow1 = std::make_shared<Flow>();
					ui_root.push_back(flow1);
					ui_root[ui_root.size() - 2]->add_child(ui_root.back());
					ui_root.back()->def_props({
						{"flow_flags", flow_down_fill},
						});

						auto label1 = std::make_shared<Label>();
						ui_root.push_back(label1);
						ui_root[ui_root.size() - 2]->add_child(ui_root.back());
						ui_root.back()->def_props({
							{"text", "Service"},
							{"color", argb_white},
							{"flow_flags", flow_flag_align_hcenter},
							});
						ui_root.pop_back();
					ui_root.pop_back();

					auto flow2 = std::make_shared<Flow>();
					ui_root.push_back(flow2);
					ui_root[ui_root.size() - 2]->add_child(ui_root.back());
					ui_root.back()->def_props({
						{"flow_flags", flow_down_fill},
						});

						auto label2 = std::make_shared<Label>();
						ui_root.push_back(label2);
						ui_root[ui_root.size() - 2]->add_child(ui_root.back());
						ui_root.back()->def_props({
							{"text", "Mailbox"},
							{"color", argb_white},
							{"flow_flags", flow_flag_align_hcenter},
							});
						ui_root.pop_back();
					ui_root.pop_back();

					auto flow3 = std::make_shared<Flow>();
					ui_root.push_back(flow3);
					ui_root[ui_root.size() - 2]->add_child(ui_root.back());
					ui_root.back()->def_props({
						{"flow_flags", flow_down_fill},
						});

						auto label3 = std::make_shared<Label>();
						ui_root.push_back(label3);
						ui_root[ui_root.size() - 2]->add_child(ui_root.back());
						ui_root.back()->def_props({
							{"text", "Info"},
							{"color", argb_white},
							{"flow_flags", flow_flag_align_hcenter},
							});
						ui_root.pop_back();
					ui_root.pop_back();
				ui_root.pop_back();
			ui_root.pop_back();
		ui_root.pop_back();

	auto s = window->pref_size();
	window->change(150, 150, s.m_w, s.m_h);

	//add to my GUI screen
	add_front(window);

	//event loop
	auto old_entries = std::vector<std::string>{};
	auto old_labels = std::vector<std::shared_ptr<Label>>{};
	while (m_running)
	{
		auto msg = mbox->read(std::chrono::milliseconds(100));
		if (msg)
		{
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
		else
		{
			//any changes to service directory
			auto entries = global_router->enquire("");
			if (entries != old_entries)
			{
				old_entries = entries;
				for (auto &view : old_labels) view->sub();
				old_labels.clear();
				for (auto &e : entries)
				{
					auto fields = split_string(e, ",");
					auto label1 = std::make_shared<Label>();
					auto label2 = std::make_shared<Label>();
					auto label3 = std::make_shared<Label>();
					old_labels.push_back(label1);
					old_labels.push_back(label2);
					old_labels.push_back(label3);
					label1->def_props({
						{"text", fields[0]},
						{"border", -1}
						});
					label2->def_props({
						{"text", fields[1]},
						{"border", -1},
						});
					label3->def_props({
						{"text", fields[2]},
						{"border", -1},
						});
					flow1->add_child(label1);
					flow2->add_child(label2);
					flow3->add_child(label3);
					s = window->pref_size();
					auto p = window->get_pos();
					window->change_dirty(p.m_x, p.m_y, s.m_w, s.m_h);
				}
			}
		}
	}
}
