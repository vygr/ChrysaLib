#ifndef TASK_H
#define TASK_H

#include "../mail/router.h"

extern std::unique_ptr<Router> global_router;

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
	Task()
		: m_net_id(global_router->alloc())
	{}
	virtual ~Task()
	{
		//free the task mailbox
		global_router->free(m_net_id);
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
protected:
	virtual void run() = 0;
	const Net_ID m_net_id;
	std::thread m_thread;
};

#endif
