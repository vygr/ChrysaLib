#include "task.h"
#include "kernel_service.h"

extern std::thread::id global_kernel_thread_id;

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

//helpers

void Task::exit()
{
	//send task stop request
	//kernel will exit !!!
	auto msg = std::make_shared<Msg>(sizeof(Kernel_Service::Event));
	auto event_body = (Kernel_Service::Event*)msg->begin();
	msg->set_dest(Net_ID(global_router->get_dev_id(), Mailbox_ID{0}));
	event_body->m_evt = Kernel_Service::evt_exit;
	global_router->send(msg);
}

Net_ID Task::start_task(Task *task)
{
	//send task start request
	//kernel will call start_thread
	auto reply_id = global_router->alloc();
	auto reply_mbox = global_router->validate(reply_id);
	auto msg = std::make_shared<Msg>(sizeof(Kernel_Service::Event_start_task));
	auto event_body = (Kernel_Service::Event_start_task*)msg->begin();
	msg->set_dest(Net_ID(global_router->get_dev_id(), Mailbox_ID{0}));
	event_body->m_evt = Kernel_Service::evt_start_task;
	event_body->m_reply = reply_id;
	event_body->m_task = task;
	global_router->send(msg);
	//wait for reply
	auto reply = reply_mbox->read();
	global_router->free(reply_id);
	auto reply_body = (Kernel_Service::start_task_reply*)reply->begin();
	return reply_body->m_task;
}

void Task::stop_task()
{
	//send task stop request
	//kernel will call stop_thread and join_thread
	auto msg = std::make_shared<Msg>(sizeof(Kernel_Service::Event_stop_task));
	auto event_body = (Kernel_Service::Event_stop_task*)msg->begin();
	msg->set_dest(Net_ID(global_router->get_dev_id(), Mailbox_ID{0}));
	event_body->m_evt = Kernel_Service::evt_stop_task;
	global_router->send(msg);
}

void Task::callback(std::function<void()> callback)
{
	//kernel will call function in its thread
	if (std::this_thread::get_id() == global_kernel_thread_id)
	{
		//we are the kernel thread !
		callback();
	}
	else
	{
		//send callback request
		Mbox<void*> wake_mbox;
		auto msg = std::make_shared<Msg>(sizeof(Kernel_Service::Event_callback));
		auto event_body = (Kernel_Service::Event_callback*)msg->begin();
		msg->set_dest(Net_ID(global_router->get_dev_id(), Mailbox_ID{0}));
		event_body->m_evt = Kernel_Service::evt_callback;
		event_body->m_wake_mbox = &wake_mbox;
		event_body->m_callback = callback;
		global_router->send(msg);
		//wait for wake
		wake_mbox.read();
	}
}
