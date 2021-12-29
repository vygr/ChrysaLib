#ifndef TASK_H
#define TASK_H

#include "../mail/router.h"

//task class, thread executes the run method
class Task
{
public:
	enum
	{
		evt_exit, //must be first !
	};
	struct Event
	{
		uint32_t m_evt;
	};
	Task(Router &router)
		: m_net_id(router.alloc())
		, m_router(router)
	{}
	virtual ~Task()
	{
		//free the task mailbox
		m_router.free(m_net_id);
	}
	//responce handling
	void start_thread()
	{
		m_running = true;
		m_thread = std::thread(&Task::run, this);
	}
	void join_thread() { if (m_thread.joinable()) m_thread.join(); }
	void stop_thread();
	const Net_ID &get_id() const { return m_net_id; }
	bool m_running = false;
	//helper methods
	Net_ID start_task(Task *task);
	void stop_task();
protected:
	virtual void run() = 0;
	const Net_ID m_net_id;
	Router &m_router;
	std::thread m_thread;
};

#endif
