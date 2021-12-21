#include "service.h"

//////////
// service
//////////

void Service::stop_thread()
{
	if (!m_running) return;
	//set stop_thread flag
	m_running = false;
	//wake the thread
	auto msg = std::make_shared<Msg>(sizeof(Event));
	msg->set_dest(m_net_id);
	auto msg_struct = (Event*)msg->begin();
	msg_struct->m_evt = evt_exit;
	m_router.send(msg);
}
