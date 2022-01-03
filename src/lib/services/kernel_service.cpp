#include "kernel_service.h"
#include "task.h"
#include <iostream>
#include <sstream>

/////////
// kernel
/////////

void Kernel_Service::run()
{
	//get my mailbox address, id was allocated in the constructor
	auto mbox = m_router->validate(m_net_id);
	auto entry = m_router->declare(m_net_id, "kernel", "Kernel_Service v0.1");

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
			if (m_router->update_route(*msg->m_data)
				&& m_router->update_dir(*msg->m_data))
			{
				//new session so flood to peers
				auto event_body = (Event_directory*)body;
				auto via = event_body->m_via;
				//fill in the new via and increment the distance as we flood out !
				event_body->m_via = m_router->get_dev_id();
				event_body->m_hops++;
				for (auto &peer : m_router->get_peers())
				{
					//don't send to peer who sent it to me !
					if (peer == via) continue;
					auto flood_msg = std::make_shared<Msg>(msg->m_data);
					flood_msg->set_dest(Net_ID(peer, Mailbox_ID{0}));
					m_router->send(flood_msg);
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
			m_router->send(reply);
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
			void *wake = nullptr;
			event_body->m_callback();
			event_body->m_mbox->post(wake);
			break;
		}
		default:
			break;
		}
	}

	//forget myself
	m_router->forget(entry);
}
