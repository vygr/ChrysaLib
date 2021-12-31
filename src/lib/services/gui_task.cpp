#include "gui_task.h"
#include "kernel_service.h"
#include "gui_service.h"
#include "../gui/colors.h"
#include "../gui/backdrop.h"
#include "../gui/window.h"
#include "../gui/flow.h"
#include "../gui/grid.h"
#include "../gui/title.h"
#include "../gui/button.h"
#include "../gui/scroll.h"

///////////
// gui task
///////////

void GUI_Task::run()
{
	//get my mailbox address, id was allocated in the constructor
	auto mbox = m_router.validate(m_net_id);

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
	auto main_widget = std::make_shared<Button>();

	window_flow->def_prop("flow_flags", std::make_shared<Property>(flow_down_fill));
	title_flow->def_prop("flow_flags", std::make_shared<Property>(flow_left_fill));
	button_grid->def_prop("grid_height", std::make_shared<Property>(1));
	title->def_prop("text", std::make_shared<Property>("Some Test Text"));
	close_button->def_prop("text", std::make_shared<Property>("X"));
	min_button->def_prop("text", std::make_shared<Property>("-"));
	max_button->def_prop("text", std::make_shared<Property>("+"));
	scroll->def_prop("min_width", std::make_shared<Property>(256))
		->def_prop("min_height", std::make_shared<Property>(256));
	main_widget->def_prop("text", std::make_shared<Property>("main_widget"))
		->def_prop("min_width", std::make_shared<Property>(512))
		->def_prop("min_height", std::make_shared<Property>(512));

	window->add_child(window_flow);
	window_flow->add_child(title_flow)->add_child(scroll);
	title_flow->add_child(button_grid)->add_child(title);
	button_grid->add_child(min_button)->add_child(max_button)->add_child(close_button);
	scroll->add_child(main_widget);
	main_widget->change(0, 0, 512, 512);
	auto s = window->pref_size();
	window->change(107, 107, s.m_w, s.m_h);

	//add to my GUI screen
	add_front(window);

	//event loop
	while (m_running)
	{
		auto msg = mbox->read();
		if (msg)
		{
			auto evt = (Event*)msg->begin();
			switch (evt->m_evt)
			{
			default:
				break;
			}
		}
	}
}

//helpers

void GUI_Task::add_front(std::shared_ptr<View> view)
{
	//message to my GUI
	view->m_owner = m_net_id;
	auto services = m_router.enquire("gui");
	auto fields = split_string(services[0], ",");
	auto service_id = Net_ID::from_string(fields[1]);
	auto reply_id = m_router.alloc();
	auto reply_mbox = m_router.validate(reply_id);
	auto msg = std::make_shared<Msg>(sizeof(GUI_Service::Event_add_front));
	auto msg_struct = (GUI_Service::Event_add_front*)msg->begin();
	msg->set_dest(service_id);
	msg_struct->m_evt = GUI_Service::evt_add_front;
	msg_struct->m_reply = reply_id;
	msg_struct->m_view = view;
	m_router.send(msg);
	//wait for reply
	reply_mbox->read();
	m_router.free(reply_id);
}

void GUI_Task::add_back(std::shared_ptr<View> view)
{
	//message to my GUI
	view->m_owner = m_net_id;
	auto services = m_router.enquire("gui");
	auto fields = split_string(services[0], ",");
	auto service_id = Net_ID::from_string(fields[1]);
	auto reply_id = m_router.alloc();
	auto reply_mbox = m_router.validate(reply_id);
	auto msg = std::make_shared<Msg>(sizeof(GUI_Service::Event_add_back));
	auto msg_struct = (GUI_Service::Event_add_back*)msg->begin();
	msg->set_dest(service_id);
	msg_struct->m_evt = GUI_Service::evt_add_back;
	msg_struct->m_reply = reply_id;
	msg_struct->m_view = view;
	m_router.send(msg);
	//wait for reply
	reply_mbox->read();
	m_router.free(reply_id);
}

void GUI_Task::sub(std::shared_ptr<View> view)
{
	//message to my GUI
	view->m_owner = m_net_id;
	auto services = m_router.enquire("gui");
	auto fields = split_string(services[0], ",");
	auto service_id = Net_ID::from_string(fields[1]);
	auto reply_id = m_router.alloc();
	auto reply_mbox = m_router.validate(reply_id);
	auto msg = std::make_shared<Msg>(sizeof(GUI_Service::Event_sub));
	auto msg_struct = (GUI_Service::Event_sub*)msg->begin();
	msg->set_dest(service_id);
	msg_struct->m_evt = GUI_Service::evt_sub;
	msg_struct->m_reply = reply_id;
	msg_struct->m_view = view;
	m_router.send(msg);
	//wait for reply
	reply_mbox->read();
	m_router.free(reply_id);
}
