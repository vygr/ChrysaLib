#ifndef GUI_TASK_H
#define GUI_TASK_H

#include "task.h"
#include "../gui/view.h"

//task
class GUI_Task : public Task
{
public:
	GUI_Task()
		: Task()
	{}
	enum
	{
		locate_center,
		locate_top,
		locate_left,
		locate_bottom,
		locate_right,
	};
	//helper methods
	Net_ID my_gui();
	void add_front(std::shared_ptr<View> view);
	void add_back(std::shared_ptr<View> view);
	void sub(std::shared_ptr<View> view);
	view_bounds locate(int32_t w, int32_t h, int32_t pos = locate_center);
	view_bounds locate(const view_size &size, int32_t pos = locate_center)
		{ return locate(size.m_w, size.m_h, pos); }
};

#endif
