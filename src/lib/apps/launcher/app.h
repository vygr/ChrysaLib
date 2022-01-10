#ifndef LAUNCHER_APP_H
#define LAUNCHER_APP_H

#include "../../services/gui_task.h"

//task
class Launcher_App : public GUI_Task
{
public:
	Launcher_App()
		: GUI_Task()
	{}
	static std::shared_ptr<Launcher_App> create() { return std::make_shared<Launcher_App>(); }
	void run() override;
};

#endif
