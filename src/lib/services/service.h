#ifndef SERVICE_H
#define SERVICE_H

#include "../mail/router.h"

//service class, thread executes the run method
class Service
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
	Service(Router &router)
		: m_net_id(router.alloc())
		, m_router(router)
	{}
	virtual ~Service()
	{
		//free the service mailbox
		m_router.free(m_net_id);
	}
	//responce handling
	void start_thread() { m_thread = std::thread(&Service::run, this); }
	void join_thread() { if (m_thread.joinable()) m_thread.join(); }
	void stop_thread();
	const Net_ID &get_id() const { return m_net_id; }
	bool m_running = false;
protected:
	virtual void run() = 0;
	const Net_ID m_net_id;
	Router &m_router;
	std::thread m_thread;
};

#endif
