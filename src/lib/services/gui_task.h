#ifndef GUI_TASK_H
#define GUI_TASK_H

#include "task.h"

class View;

//task
class GUI_Task : public Task
{
public:
	GUI_Task(Router &router)
		: Task(router)
	{}
	void run() override;
	//helper methods
	void add_front(std::shared_ptr<View> view);
	void add_back(std::shared_ptr<View> view);
	void sub(std::shared_ptr<View> view);
};

#endif
