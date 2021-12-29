#ifndef TASK_H
#define TASK_H

#include "service.h"

class View;

//task
class Task : public Service
{
public:
	enum
	{
		evt_exit, //must be first !
	};
	Task(Router &router)
		: Service(router)
	{}
	void run() override;
	//helper methods
	Net_ID start_task(Task *task);
	void stop_task();
	void add_front(std::shared_ptr<View> view);
	void add_back(std::shared_ptr<View> view);
	void sub(std::shared_ptr<View> view);
};

#endif
