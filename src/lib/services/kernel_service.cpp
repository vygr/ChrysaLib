#include "kernel_service.h"
#include "task.h"
#include <iostream>
#include <sstream>

extern std::thread::id global_kernel_thread_id;

/////////
// kernel
/////////

void Kernel_Service::run()
{
	//get my mailbox address, id was allocated in the constructor
	auto mbox = global_router->validate(m_net_id);
	auto entry = global_router->declare(m_net_id, "kernel", "Kernel_Service v0.1");

	//event loop
	while (m_running)
	{
		auto msg = mbox->read();
		auto body = (Event*)msg->begin();
		switch (body->m_evt)
		{
		case evt_exit:
		{
			m_running = false;
			break;
		}
		case evt_directory:
		{
			//directory update, flood filling
			if (global_router->update_route(*msg->m_data)
				&& global_router->update_dir(*msg->m_data))
			{
				//new session so flood to peers
				auto event_body = (Event_directory*)body;
				auto via = event_body->m_via;
				//fill in the new via and increment the distance as we flood out !
				event_body->m_via = global_router->get_dev_id();
				event_body->m_hops++;
				for (auto &peer : global_router->get_peers())
				{
					//don't send to peer who sent it to me !
					if (peer == via) continue;
					auto flood_msg = std::make_shared<Msg>(msg->m_data);
					flood_msg->set_dest(Net_ID(peer, Mailbox_ID{0}));
					global_router->send(flood_msg);
				}
			}
			break;
		}
		case evt_start_task:
		{
			//start task
			auto event_body = (Event_start_task*)body;
			event_body->m_task->start_thread();
			auto reply = std::make_shared<Msg>(sizeof(start_task_reply));
			auto reply_body = (start_task_reply*)reply->begin();
			reply->set_dest(event_body->m_reply);
			reply_body->m_task = event_body->m_task->get_id();
			global_router->send(reply);
			break;
		}
		case evt_stop_task:
		{
			//stop task
			auto event_body = (Event_stop_task*)body;
			event_body->m_task->stop_thread();
			event_body->m_task->join_thread();
			break;
		}
		case evt_callback:
		{
			//callback
			auto event_body = (Event_callback*)body;
			event_body->m_callback();
			event_body->m_sync->wake();
			break;
		}
		default:
			break;
		}
	}

	//forget myself
	global_router->forget(entry);
}

void Kernel_Service::exit()
{
	//send task stop request
	//kernel will exit !!!
	auto msg = std::make_shared<Msg>(sizeof(Kernel_Service::Event));
	auto event_body = (Kernel_Service::Event*)msg->begin();
	msg->set_dest(Net_ID(global_router->get_dev_id(), Mailbox_ID{0}));
	event_body->m_evt = Kernel_Service::evt_exit;
	global_router->send(msg);
}

Net_ID Kernel_Service::start_task(Task *task)
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

void Kernel_Service::stop_task()
{
	//send task stop request
	//kernel will call stop_thread and join_thread
	auto msg = std::make_shared<Msg>(sizeof(Kernel_Service::Event_stop_task));
	auto event_body = (Kernel_Service::Event_stop_task*)msg->begin();
	msg->set_dest(Net_ID(global_router->get_dev_id(), Mailbox_ID{0}));
	event_body->m_evt = Kernel_Service::evt_stop_task;
	global_router->send(msg);
}

void Kernel_Service::callback(std::function<void()> callback)
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
		Sync sync;
		auto msg = std::make_shared<Msg>(sizeof(Kernel_Service::Event_callback));
		auto event_body = (Kernel_Service::Event_callback*)msg->begin();
		msg->set_dest(Net_ID(global_router->get_dev_id(), Mailbox_ID{0}));
		event_body->m_evt = Kernel_Service::evt_callback;
		event_body->m_sync = &sync;
		event_body->m_callback = callback;
		global_router->send(msg);
		//wait for wake
		sync.wait();
	}
}
