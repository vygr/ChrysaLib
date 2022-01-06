#ifndef TEST_TASK2_H
#define TEST_TASK2_H

#include "gui_task.h"

//task
class Test_2 : public GUI_Task
{
public:
	Test_2()
		: GUI_Task()
	{}
	void run() override;
};

#endif
