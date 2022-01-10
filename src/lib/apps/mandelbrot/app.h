#ifndef MANDELBROT_APP_H
#define MANDELBROT_APP_H

#include "../../services/gui_task.h"
#include "../../utils/threadpool.h"
#include "../../task/farm.h"

//task
class Mandelbrot_App : public GUI_Task
{
private:
	const uint32_t CANVAS_WIDTH = 800;
	const uint32_t CANVAS_HEIGHT = 800;
	const uint32_t CANVAS_SCALE = 2;
	const uint32_t UPDATE_TIMEOUT = 100;
	const uint32_t JOB_TIMEOUT = 5000;
	const uint32_t JOB_LIMIT = 16;
public:
	Mandelbrot_App()
		: GUI_Task()
		, m_thread_pool(std::make_unique<ThreadPool>(JOB_LIMIT))
	{}
	void run() override;
private:
	struct Mandelbrot_Job : public Farm::Job
	{
		Net_ID m_reply;
		uint32_t m_x;
		uint32_t m_y;
		uint32_t m_x1;
		uint32_t m_y1;
		uint32_t m_cw;
		uint32_t m_ch;
		double m_cx;
		double m_cy;
		double m_z;
	};
	struct Mandelbrot_Job_reply : public Farm::Job
	{
		uint32_t m_x;
		uint32_t m_y;
		uint32_t m_x1;
		uint32_t m_y1;
		uint8_t m_data[];
	};
	enum
	{
		select_main,
		select_reply,
		select_worker,
		select_timer,
		select_size,
	};
	uint8_t depth(double x0, double y0) const;
	void reset();
	std::unique_ptr<ThreadPool> m_thread_pool;
	std::unique_ptr<Farm> m_farm;
	std::vector<Net_ID> m_select;
	std::string m_entry;
	bool m_dirty = false;
	double m_zoom = 1.0;
	double m_cx = -0.5;
	double m_cy = 0.0;
};

#endif
