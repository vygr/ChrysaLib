#include "kernel_service.h"
#include "task.h"
#include <iostream>
#include <sstream>
#include <algorithm>

extern std::thread::id global_kernel_thread_id;
extern uint32_t arg_v;

/////////
// kernel
/////////

void Kernel_Service::run()
{
	//get my mailbox address, id was allocated in the constructor
	auto mbox = global_router->validate(m_net_id);
	auto entry = std::string{};
	if (arg_v > 1) entry = global_router->declare(m_net_id, "*Kernel", "Kernel_Service v0.1");

	//current time
	auto now = std::chrono::high_resolution_clock::now();

	//event loop
	while (m_running)
	{
		//read and wake for next timer time
		auto delay = std::chrono::milliseconds(1000000);
		if (!m_timer.empty())
		{
			auto time = ((Event_timed_mail*)m_timer.front()->begin())->m_time;
			delay = std::chrono::duration_cast<std::chrono::milliseconds>(time - now);
			delay = std::min(delay, std::chrono::milliseconds(1));
		}
		auto msg = mbox->read(delay);
		if (msg)
		{
			auto body = (Event*)msg->begin();
			switch (body->m_evt)
			{
			case evt_exit:
			{
				m_running = false;
				break;
			}
			case evt_ping:
			{
				//directory update, flood filling
				if (global_router->update_route(*msg->m_data)
					&& global_router->update_dir(*msg->m_data))
				{
					//new session so flood to peers
					auto event_body = (Event_ping*)body;
					auto via = event_body->m_via;
					//fill in the new via and increment the distance as we flood out !
					event_body->m_via = global_router->get_node_id();
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
				m_tasks.push_back(event_body->m_task);
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
				auto itr = std::find(begin(m_tasks), end(m_tasks), event_body->m_task);
				if (itr == end(m_tasks)) break;
				event_body->m_task->stop_thread();
				break;
			}
			case evt_join_task:
			{
				//stop task
				auto event_body = (Event_stop_task*)body;
				auto itr = std::find(begin(m_tasks), end(m_tasks), event_body->m_task);
				if (itr == end(m_tasks)) break;
				m_tasks.erase(itr);
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
			case evt_timed_mail:
			{
				//timed mail
				auto event_body = (Event_timed_mail*)body;
				if (event_body->m_timeout != std::chrono::milliseconds(0))
				{
					//insert timer
					auto time = std::chrono::high_resolution_clock::now()
						+ event_body->m_timeout;
					event_body->m_time = time;
					auto itr = std::find_if(begin(m_timer), end(m_timer), [&] (auto &msg)
					{
						auto body = (Event_timed_mail*)msg->begin();
						return body->m_time <= time;
					});
					m_timer.insert(itr, msg);
				}
				else
				{
					//remove timer
					auto &mb = event_body->m_mbox;
					auto itr = std::find_if(begin(m_timer), end(m_timer), [&] (auto &msg)
					{
						auto body = (Event_timed_mail*)msg->begin();
						return body->m_mbox == mb;
					});
					if (itr != end(m_timer)) m_timer.erase(itr);
				}
				break;
			}
			default:
				break;
			}
		}

		//check timer list
		now = std::chrono::high_resolution_clock::now();
		for (auto itr = begin(m_timer); itr != end(m_timer);)
		{
			auto tmsg = *itr;
			auto body = (Event_timed_mail*)tmsg->begin();
			if (body->m_time <= now)
			{
				itr = m_timer.erase(itr);
				tmsg->set_dest(body->m_mbox);
				global_router->send(tmsg);
			}
			else ++itr;
		}
	}

	//forget myself
	if (arg_v > 1) global_router->forget(entry);
}

void Kernel_Service::exit()
{
	//send task stop request
	//kernel will exit !!!
	auto msg = std::make_shared<Msg>(sizeof(Kernel_Service::Event));
	auto event_body = (Kernel_Service::Event*)msg->begin();
	msg->set_dest(Net_ID(global_router->get_node_id(), Mailbox_ID{0}));
	event_body->m_evt = Kernel_Service::evt_exit;
	global_router->send(msg);
}

Net_ID Kernel_Service::start_task(std::shared_ptr<Task> task)
{
	//send task start request
	//kernel will call start_thread
	auto reply_id = global_router->alloc();
	auto reply_mbox = global_router->validate(reply_id);
	auto msg = std::make_shared<Msg>(sizeof(Kernel_Service::Event_start_task));
	auto event_body = (Kernel_Service::Event_start_task*)msg->begin();
	msg->set_dest(Net_ID(global_router->get_node_id(), Mailbox_ID{0}));
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

void Kernel_Service::stop_task(std::shared_ptr<Task> task)
{
	//send task stop request
	//kernel will call stop_thread
	auto msg = std::make_shared<Msg>(sizeof(Kernel_Service::Event_stop_task));
	auto event_body = (Kernel_Service::Event_stop_task*)msg->begin();
	msg->set_dest(Net_ID(global_router->get_node_id(), Mailbox_ID{0}));
	event_body->m_evt = Kernel_Service::evt_stop_task;
	event_body->m_task = task;
	global_router->send(msg);
}

void Kernel_Service::join_task(std::shared_ptr<Task> task)
{
	//send task join request
	//kernel will call join_thread
	auto msg = std::make_shared<Msg>(sizeof(Kernel_Service::Event_stop_task));
	auto event_body = (Kernel_Service::Event_stop_task*)msg->begin();
	msg->set_dest(Net_ID(global_router->get_node_id(), Mailbox_ID{0}));
	event_body->m_evt = Kernel_Service::evt_stop_task;
	event_body->m_task = task;
	global_router->send(msg);
}

void Kernel_Service::timed_mail(const Net_ID &reply, std::chrono::milliseconds timeout, uint64_t id)
{
	//timed mail request
	auto msg = std::make_shared<Msg>(sizeof(Kernel_Service::Event_timed_mail));
	auto event_body = (Kernel_Service::Event_timed_mail*)msg->begin();
	msg->set_dest(Net_ID(global_router->get_node_id(), Mailbox_ID{0}));
	event_body->m_evt = Kernel_Service::evt_timed_mail;
	event_body->m_mbox = reply;
	event_body->m_timeout = timeout;
	event_body->m_id = id;
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
		msg->set_dest(Net_ID(global_router->get_node_id(), Mailbox_ID{0}));
		event_body->m_evt = Kernel_Service::evt_callback;
		event_body->m_sync = &sync;
		event_body->m_callback = callback;
		global_router->send(msg);
		//wait for wake
		sync.wait();
	}
}
