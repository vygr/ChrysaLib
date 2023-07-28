#ifndef TASK_H
#define TASK_H

#include "../mail/router.h"

extern std::unique_ptr<Router> global_router;

//task class, thread executes the run method
class Task : public std::enable_shared_from_this<Task>
{
public:
	enum
	{
		evt_exit, //must be first !
	};
	struct Event
	{
		uint64_t m_evt;
	};
	Task()
		: m_net_id(global_router->alloc())
	{}
	virtual ~Task()
	{
		//free the task mailbox
		global_router->free(m_net_id);
	}
	//response handling
	void start_thread();
	void stop_thread();
	void join_thread();
	const Net_ID &get_id() const { return m_net_id; }
	bool m_running = false;
protected:
	std::vector<Net_ID> alloc_select(uint32_t size);
	void free_select(std::vector<Net_ID> &select);
	void run_then_join();
	virtual void run() = 0;
	const Net_ID m_net_id;
	std::thread m_thread;
};

#endif
