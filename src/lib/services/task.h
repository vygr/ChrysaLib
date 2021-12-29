#ifndef TASK_H
#define TASK_H

#include "service.h"

//task
class Task : public Service
{
public:
	Task(Router &router)
		: Service(router)
	{}
	//helper methods
	Net_ID start_task(Task *task);
	void stop_task();
};

#endif
