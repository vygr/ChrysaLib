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
	static std::shared_ptr<Canvas_App> create() { return std::make_shared<Canvas_App>(); }
	void run() override;
};

#endif
