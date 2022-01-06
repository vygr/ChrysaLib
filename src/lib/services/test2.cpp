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

	//test UI app
	auto window = std::make_shared<Window>();
	auto window_flow = std::make_shared<Flow>();
	auto title_flow = std::make_shared<Flow>();
	auto button_grid = std::make_shared<Grid>();
	auto title = std::make_shared<Title>();
	auto close_button = std::make_shared<Button>();
	auto main_widget = std::make_shared<Flow>();
	auto flow1 = std::make_shared<Flow>();
	auto flow2 = std::make_shared<Flow>();
	auto flow3 = std::make_shared<Flow>();

	window_flow->def_prop("flow_flags", std::make_shared<Property>(flow_down_fill));
	title_flow->def_prop("flow_flags", std::make_shared<Property>(flow_left_fill));
	button_grid->def_prop("grid_height", std::make_shared<Property>(1))
		->def_prop("font", std::make_shared<Property>(Font::open("fonts/Entypo.ctf", 22)));
	title->def_prop("text", std::make_shared<Property>("Services"));
	close_button->def_prop("text", std::make_shared<Property>(to_utf8(0xea19)));
	main_widget->def_prop("flow_flags", std::make_shared<Property>(flow_right_fill));

	flow1->def_prop("flow_flags", std::make_shared<Property>(flow_down_fill));
	flow2->def_prop("flow_flags", std::make_shared<Property>(flow_down_fill));
	flow3->def_prop("flow_flags", std::make_shared<Property>(flow_down_fill));
	auto label1 = std::make_shared<Label>();
	auto label2 = std::make_shared<Label>();
	auto label3 = std::make_shared<Label>();
	label1->def_prop("text", std::make_shared<Property>("Service"))
		->def_prop("color", std::make_shared<Property>(argb_white))
		->def_prop("flow_flags", std::make_shared<Property>(flow_flag_align_hcenter));
	label2->def_prop("text", std::make_shared<Property>("Mailbox"))
		->def_prop("color", std::make_shared<Property>(argb_white))
		->def_prop("flow_flags", std::make_shared<Property>(flow_flag_align_hcenter));
	label3->def_prop("text", std::make_shared<Property>("Info"))
		->def_prop("color", std::make_shared<Property>(argb_white))
		->def_prop("flow_flags", std::make_shared<Property>(flow_flag_align_hcenter));

	window->add_child(window_flow);
	window_flow->add_child(title_flow)->add_child(main_widget);
	title_flow->add_child(button_grid)->add_child(title);
	button_grid->add_child(close_button);
	main_widget->add_child(flow1)->add_child(flow2)->add_child(flow3);
	flow1->add_child(label1);
	flow2->add_child(label2);
	flow3->add_child(label3);

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
					label1->def_prop("text", std::make_shared<Property>(fields[0]));
					label2->def_prop("text", std::make_shared<Property>(fields[1]));
					label3->def_prop("text", std::make_shared<Property>(fields[2]));
					flow1->add_child(label1);
					flow2->add_child(label2);
					flow3->add_child(label3);
					s = window->pref_size();
					window->change_dirty(150, 150, s.m_w, s.m_h);
				}
			}
		}
	}
}
