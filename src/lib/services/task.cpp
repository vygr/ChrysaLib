#include "task.h"
#include "kernel_service.h"

///////
// task
///////

void Task::stop_thread()
{
	if (!m_running) return;
	//set stop_thread flag
	m_running = false;
	//wake the thread
	auto msg = std::make_shared<Msg>(sizeof(Event));
	msg->set_dest(m_net_id);
	auto event_body = (Event*)msg->begin();
	event_body->m_evt = evt_exit;
	global_router->send(msg);
}
