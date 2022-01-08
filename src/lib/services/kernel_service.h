#ifndef KERNEL_H
#define KERNEL_H

#include "service.h"
#include <list>

class Task;

//kernel service.
//this is the very first service to be run, therfore mailbox id 0.
//this is critical as routers talk to each other via this kernel service.
//currently the only thing the kernel service does is maintain the distributed
//service directory.
class Kernel_Service : public Service
{
public:
	enum
	{
		evt_exit, //must be first !
		evt_directory,
		evt_start_task,
		evt_stop_task,
		evt_callback,
		evt_timed_mail,
	};
	struct Event_directory : public Event
	{
		Net_ID m_src;
		Dev_ID m_via;
		uint32_t m_hops;
		char m_data[];
	};
	struct Event_start_task : public Event
	{
		Net_ID m_reply;
		Task *m_task;
	};
	struct start_task_reply
	{
		Net_ID m_task;
	};
	struct Event_stop_task : public Event
	{
		Task *m_task;
	};
	struct Event_callback : public Event
	{
		Sync *m_sync;
		std::function<void()> m_callback;
	};
	struct Event_timed_mail : public Event
	{
		Net_ID m_mbox;
		std::chrono::high_resolution_clock::time_point m_time;
		std::chrono::milliseconds m_timeout;
		uint64_t m_id;
	};
	Kernel_Service()
		: Service()
	{}
	void run() override;
	static void callback(std::function<void()> callback);
	static void exit();
	static Net_ID start_task(Task *task);
	static void stop_task();
	static void timed_mail(const Net_ID &reply, std::chrono::milliseconds timeout, uint64_t id);
	std::list<std::shared_ptr<Msg>> m_timer;
};

#endif
