#ifndef MANDELBROT_APP_H
#define MANDELBROT_APP_H

#include "../../services/gui_task.h"

//task
class Mandelbrot_App : public GUI_Task
{
public:
	Mandelbrot_App()
		: GUI_Task()
	{}
	void run() override;
};

#endif
