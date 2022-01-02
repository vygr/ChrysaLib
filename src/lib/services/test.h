#ifndef TEST_TASK_H
#define TEST_TASK_H

#include "gui_task.h"

//task
class Test_Task : public GUI_Task
{
public:
	Test_Task()
		: GUI_Task()
	{}
	void run() override;
};

#endif
