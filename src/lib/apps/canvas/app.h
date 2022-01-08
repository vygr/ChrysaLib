#ifndef CANVAS_APP_H
#define CANVAS_APP_H

#include "../../services/gui_task.h"

//task
class Canvas_App : public GUI_Task
{
public:
	Canvas_App()
		: GUI_Task()
	{}
	void run() override;
};

#endif
