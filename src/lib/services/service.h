#ifndef SERVICE_H
#define SERVICE_H

#include "task.h"

//service
class Service : public Task
{
public:
	Service(Router &router)
		: Task(router)
	{}
};

#endif
