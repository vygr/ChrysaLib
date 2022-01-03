#ifndef KERNEL_H
#define KERNEL_H

#include "service.h"

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
		Mbox<void*> *m_mbox;
		std::function<void()> m_callback;
	};
	Kernel_Service()
		: Service()
	{}
	void run() override;
};

#endif
