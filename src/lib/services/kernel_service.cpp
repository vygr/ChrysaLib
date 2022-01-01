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
	auto mbox = m_router.validate(m_net_id);
	auto entry = m_router.declare(m_net_id, "kernel", "Kernel_Service v0.1");

	//event loop
	while (m_running)
	{
		auto msg = mbox->read();
		auto evt = (Event*)msg->begin();
		switch (evt->m_evt)
		{
		case evt_directory:
		{
			//directory update, flood filling
			if (m_router.update_route(*msg->m_data)
				&& m_router.update_dir(*msg->m_data))
			{
				//new session so flood to peers
				auto body_struct = (Event_directory*)evt;
				auto via = body_struct->m_via;
				//fill in the new via and increment the distance as we flood out !
				body_struct->m_via = m_router.get_dev_id();
				body_struct->m_hops++;
				for (auto &peer : m_router.get_peers())
				{
					//don't send to peer who sent it to me !
					if (peer == via) continue;
					auto flood_msg = std::make_shared<Msg>(msg->m_data);
					flood_msg->set_dest(Net_ID(peer, Mailbox_ID{0}));
					m_router.send(flood_msg);
				}
			}
			break;
		}
		case evt_start_task:
		{
			//start task
			auto body_struct = (Event_start_task*)evt;
			body_struct->m_task->start_thread();
			auto reply = std::make_shared<Msg>(sizeof(start_task_reply));
			auto reply_body = (start_task_reply*)reply->begin();
			reply->set_dest(body_struct->m_reply);
			reply_body->m_task = body_struct->m_task->get_id();
			m_router.send(reply);
			break;
		}
		case evt_stop_task:
		{
			//stop task
			auto body_struct = (Event_stop_task*)evt;
			body_struct->m_task->stop_thread();
			body_struct->m_task->join_thread();
			break;
		}
		default:
			break;
		}
	}

	//forget myself
	m_router.forget(entry);
}
