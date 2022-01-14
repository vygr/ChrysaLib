#include "gui_task.h"
#include "kernel_service.h"
#include "gui_service.h"
#include "../gui/view.h"

///////////
// gui task
///////////

//helpers

Net_ID GUI_Task::my_gui()
{
	//return my GUI node
	if (m_gui_id != Net_ID()) return m_gui_id;
	auto filter = "gui," + global_router->get_dev_id().to_string();
	auto services = global_router->enquire(filter);
	if (services.empty()) return Net_ID();
	auto fields = split_string(services[0], ",");
	return m_gui_id = Net_ID::from_string(fields[1]);
}

void GUI_Task::add_front(std::shared_ptr<View> view)
{
	//message to my GUI
	auto service_id = my_gui();
	if (service_id == Net_ID()) return;
	view->m_owner = m_net_id;
	auto reply_id = global_router->alloc();
	auto reply_mbox = global_router->validate(reply_id);
	auto msg = std::make_shared<Msg>(sizeof(GUI_Service::Event_add_front));
	auto event_body = (GUI_Service::Event_add_front*)msg->begin();
	msg->set_dest(service_id);
	event_body->m_evt = GUI_Service::evt_add_front;
	event_body->m_reply = reply_id;
	event_body->m_view = view;
	global_router->send(msg);
	//wait for reply
	reply_mbox->read();
	global_router->free(reply_id);
}

void GUI_Task::add_back(std::shared_ptr<View> view)
{
	//message to my GUI
	auto service_id = my_gui();
	if (service_id == Net_ID()) return;
	view->m_owner = m_net_id;
	auto reply_id = global_router->alloc();
	auto reply_mbox = global_router->validate(reply_id);
	auto msg = std::make_shared<Msg>(sizeof(GUI_Service::Event_add_back));
	auto event_body = (GUI_Service::Event_add_back*)msg->begin();
	msg->set_dest(service_id);
	event_body->m_evt = GUI_Service::evt_add_back;
	event_body->m_reply = reply_id;
	event_body->m_view = view;
	global_router->send(msg);
	//wait for reply
	reply_mbox->read();
	global_router->free(reply_id);
}

void GUI_Task::sub(std::shared_ptr<View> view)
{
	//message to my GUI
	auto service_id = my_gui();
	if (service_id == Net_ID()) return;
	view->m_owner = m_net_id;
	auto reply_id = global_router->alloc();
	auto reply_mbox = global_router->validate(reply_id);
	auto msg = std::make_shared<Msg>(sizeof(GUI_Service::Event_sub));
	auto event_body = (GUI_Service::Event_sub*)msg->begin();
	msg->set_dest(service_id);
	event_body->m_evt = GUI_Service::evt_sub;
	event_body->m_reply = reply_id;
	event_body->m_view = view;
	global_router->send(msg);
	//wait for reply
	reply_mbox->read();
	global_router->free(reply_id);
}

view_bounds GUI_Task::locate(int32_t w, int32_t h, int32_t pos)
{
	//message to my GUI
	auto service_id = my_gui();
	if (service_id == Net_ID()) return view_bounds{0, 0, 0, 0};
	auto reply_id = global_router->alloc();
	auto reply_mbox = global_router->validate(reply_id);
	auto msg = std::make_shared<Msg>(sizeof(GUI_Service::Event_locate));
	auto event_body = (GUI_Service::Event_locate*)msg->begin();
	msg->set_dest(service_id);
	event_body->m_evt = GUI_Service::evt_locate;
	event_body->m_reply = reply_id;
	event_body->m_w = w;
	event_body->m_h = h;
	event_body->m_pos = pos;
	global_router->send(msg);
	//wait for reply
	auto reply = reply_mbox->read();
	global_router->free(reply_id);
	auto reply_body = (GUI_Service::locate_reply*)reply->begin();
	return reply_body->m_bounds;
}
