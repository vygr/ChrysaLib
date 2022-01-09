#ifndef MANDELBROT_APP_H
#define MANDELBROT_APP_H

#include "../../services/gui_task.h"
#include "../../utils/threadpool.h"

//task
class Mandelbrot_App : public GUI_Task
{
private:
	const uint32_t CANVAS_WIDTH = 640;
	const uint32_t CANVAS_HEIGHT = 640;
	const uint32_t CANVAS_SCALE = 2;
	const uint32_t JOB_TIMEOUT = 1000;
	const uint32_t JOB_LIMIT = 4;
public:
	Mandelbrot_App()
		: GUI_Task()
		, m_thread_pool(std::make_unique<ThreadPool>(JOB_LIMIT))
	{}
	void run() override;
private:
	struct Job
	{
		Net_ID m_reply;
		Net_ID m_worker;
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
		Net_ID m_worker;
		uint32_t m_key;
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
	struct ticket
	{
		std::shared_ptr<Msg> m_job;
		std::chrono::high_resolution_clock::time_point m_time;
	};
	uint8_t depth(double x0, double y0) const;
	void reset();
	std::vector<Net_ID> census();
	void joiners(const std::vector<Net_ID> &census);
	void leavers(const std::vector<Net_ID> &census);
	void add_worker(const Net_ID &worker);
	void sub_worker(const Net_ID &worker);
	void complete(const Net_ID &worker, uint32_t key);
	void dispatch(std::shared_ptr<Msg> job, const Net_ID &worker);
	void restart();
	void slackers();
	std::vector<Net_ID> m_workers;
	std::list<std::shared_ptr<Msg>> m_jobs_ready;
	std::map<Net_ID, std::list<ticket>> m_jobs_assigned;
	std::unique_ptr<ThreadPool> m_thread_pool;
	std::vector<Net_ID> m_select;
	std::string m_entry;
	bool m_dirty = false;
	double m_zoom = 1.0;
	double m_cx = -0.5;
	double m_cy = 0.0;
};

#endif
