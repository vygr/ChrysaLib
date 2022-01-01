#ifndef TEST_TASK_H
#define TEST_TASK_H

#include "gui_task.h"

//task
class Test_Task : public GUI_Task
{
public:
	Test_Task(Router &router)
		: GUI_Task(router)
	{}
	void run() override;
};

#endif
