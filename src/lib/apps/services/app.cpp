#include "app.h"
#include "../../gui/ui.h"
#include <cstring>

std::string to_utf8(uint32_t c);
uint32_t from_utf8(uint8_t **data);

void Services_App::run()
{
	enum
	{
		select_main,
		select_timer,
		select_size,
	};
	
	enum
	{
		event_close,
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
					{"text", "Services"}}));
				ui_end
			ui_end
			ui_flow(main_widget, ({
				{"flow_flags", flow_right_fill}}))
				ui_flow(flow1, ({
					{"flow_flags", flow_down_fill}}))
					ui_label(labe1, ({
						{"text", "Service"},
						{"color", argb_white},
						{"flow_flags", flow_flag_align_hcenter}}))
					ui_end
				ui_end
				ui_flow(flow2, ({
					{"flow_flags", flow_down_fill}}))
					ui_label(labe2, ({
						{"text", "Mailbox"},
						{"color", argb_white},
						{"flow_flags", flow_flag_align_hcenter}}))
					ui_end
				ui_end
				ui_flow(flow3, ({
					{"flow_flags", flow_down_fill}}))
					ui_label(labe3, ({
						{"text", "Info"},
						{"color", argb_white},
						{"flow_flags", flow_flag_align_hcenter}}))
					ui_end
				ui_end
			ui_end
		ui_end
	ui_end

	//add to my GUI screen
	window->change(locate(window->pref_size()));
	add_front(window);

	//event loop
	auto old_entries = std::vector<std::string>{};
	auto old_labels = std::vector<std::shared_ptr<Label>>{};
	auto select = alloc_select(select_size);
	Kernel_Service::timed_mail(select[select_timer], std::chrono::milliseconds(100), 0);
	while (m_running)
	{
		auto idx = global_router->select(select);
		auto msg = global_router->read(select[idx]);
		switch (idx)
		{
		case select_main:
		{
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
			break;
		}
		case select_timer:
		{
			//any changes to service directory
			Kernel_Service::timed_mail(select[select_timer], std::chrono::milliseconds(100), 0);
			auto entries = global_router->enquire("");
			// //filter out "kernel" services as they all have one.
			// entries.erase(std::remove_if(begin(entries), end(entries), [&] (auto &s)
			// {
			// 	return strncmp(s.c_str(), "kernel,", 7) == 0;
			// }), end(entries));
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
					auto s = window->pref_size();
					auto p = window->get_pos();
					window->change_dirty(p.m_x, p.m_y, s.m_w, s.m_h);
				}
			}
			break;
		}
		}
	}

	//tidy up
	sub(window);
	free_select(select);
}
