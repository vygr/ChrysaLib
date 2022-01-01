#include "gui_task.h"
#include "kernel_service.h"
#include "gui_service.h"
#include "../gui/view.h"

///////////
// gui task
///////////

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
