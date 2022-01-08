#ifndef MANDELBROT_APP_H
#define MANDELBROT_APP_H

#include "../../services/gui_task.h"
#include "../../utils/threadpool.h"

//task
class Mandelbrot_App : public GUI_Task
{
public:
	struct Job
	{
		Net_ID m_reply;
		std::chrono::high_resolution_clock::time_point m_time;
		uint32_t m_key;
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
	struct Job_reply
	{
		uint32_t m_key;
		uint32_t m_x;
		uint32_t m_y;
		uint32_t m_x1;
		uint32_t m_y1;
		uint8_t m_data[];
	};
	Mandelbrot_App()
		: GUI_Task()
		, m_thread_pool(std::make_unique<ThreadPool>(8))
	{}
	void run() override;
private:
	uint8_t depth(double x0, double y0) const;
	std::unique_ptr<ThreadPool> m_thread_pool;
	bool m_dirty = false;
};

#endif
