#ifndef SERVICES_APP_H
#define SERVICES_APP_H

#include "../../services/gui_task.h"

//task
class Services_App : public GUI_Task
{
public:
	Services_App()
		: GUI_Task()
	{}
	void run() override;
};

#endif
