#ifndef TEST_TASK1_H
#define TEST_TASK1_H

#include "gui_task.h"

//task
class Test_1 : public GUI_Task
{
public:
	Test_1()
		: GUI_Task()
	{}
	void run() override;
};

#endif
