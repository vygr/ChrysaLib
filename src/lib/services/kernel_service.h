#ifndef KERNEL_H
#define KERNEL_H

#include "service.h"

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
	};
	struct Event_directory : public Event
	{
		Net_ID m_src;
		Dev_ID m_via;
		uint32_t m_hops;
		char m_data[];
	};
	Kernel_Service(Router &router)
		: Service(router)
	{}
private:
	void run() override;
};

#endif
