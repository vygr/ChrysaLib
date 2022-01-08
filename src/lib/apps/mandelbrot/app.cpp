#include "app.h"
#include "../../gui/ui.h"
#include <assert.h>

std::string to_utf8(uint32_t c);
uint32_t from_utf8(uint8_t **data);

const uint32_t CANVAS_WIDTH = 640;
const uint32_t CANVAS_HEIGHT = 640;
const uint32_t CANVAS_SCALE = 2;

void Mandelbrot_App::run()
{
	ui_window(window, ({}))
		ui_flow(window_flow, ({
			{"flow_flags", flow_down_fill}}))
			ui_flow(title_flow, ({
				{"flow_flags", flow_left_fill}}))
				ui_grid(button_grid, ({
					{"grid_height", 1},
					{"font", Font::open("fonts/Entypo.ctf", 22)}}))
					ui_button(close_button, ({
						{"text", to_utf8(0xea19)}}))
					ui_end
				ui_end
				ui_title(title, ({
					{"text", "Mandelbrot"}}))
				ui_end
			ui_end
			ui_canvas(canvas, CANVAS_WIDTH, CANVAS_HEIGHT, CANVAS_SCALE, ({}))
			ui_end
		ui_end
	ui_end

	auto s = window->pref_size();
	window->change(120, 64, s.m_w, s.m_h);

	//clear canvas
	canvas->set_col(argb_black);
	canvas->fill();
	canvas->swap();

	//add to my GUI screen
	add_front(window);

	//select and init workers
	m_select = alloc_select(select_size);
	m_entry = global_router->declare(m_select[select_worker], "mandel_worker", "Mandelbrot v0.01");
	refresh_workers();

	//job que
	for (auto y = 0; y < CANVAS_HEIGHT * CANVAS_SCALE; ++y)
	{
		m_jobs_ready.emplace_back(std::make_shared<Msg>(sizeof(Job)));
		auto job_body = (Job*)m_jobs_ready.back()->begin();
		job_body->m_x = 0;
		job_body->m_y = y;
		job_body->m_x1 = CANVAS_WIDTH * CANVAS_SCALE;
		job_body->m_y1 = y + 1;
		job_body->m_cw = CANVAS_WIDTH * CANVAS_SCALE;
		job_body->m_ch = CANVAS_HEIGHT * CANVAS_SCALE;
		job_body->m_cx = -0.5;
		job_body->m_cy = 0.0;
		job_body->m_z = 1.0;
	}

	//send off some jobs
	for (auto i = 0; i < 32; ++i) dispatch_job();

	//event loop
	Kernel_Service::timed_mail(m_select[select_timer], std::chrono::milliseconds(1000), 0);
	while (m_running)
	{
		auto idx = global_router->select(m_select);
		auto msg = global_router->read(m_select[idx]);
		switch (idx)
		{
		case select_worker:
		{
			//job request, hive off to worker thread
			//note these request can come from anywhere !
			//we will gladly do the work for anyone.
			m_thread_pool->enqueue([=, msg_ref = std::move(msg)]
			{
				auto job_body = (Job*)msg_ref->begin();
				auto x = job_body->m_x;
				auto y = job_body->m_y;
				auto x1 = job_body->m_x1;
				auto y1 = job_body->m_y1;
				auto cw = job_body->m_cw;
				auto ch = job_body->m_ch;
				auto cx = job_body->m_cx;
				auto cy = job_body->m_cy;
				auto z = job_body->m_z;
				auto stride = (x1 - x);
				auto reply = std::make_shared<Msg>(sizeof(Job_reply) + stride * (y1 - y));
				auto reply_body = (Job_reply*)reply->begin();
				reply_body->m_key = job_body->m_key;
				reply_body->m_x = x;
				reply_body->m_y = y;
				reply_body->m_x1 = x1;
				reply_body->m_y1 = y1;
				for (auto ry = y; ry < y1; ++ry)
				{
					for (auto rx = x; rx < x1; ++rx)
					{
						auto dx = (((((double)rx - (cw / 2.0)) * 2.0) / cw) * z) + cx;
						auto dy = (((((double)ry - (ch / 2.0)) * 2.0) / ch) * z) + cy;
						reply_body->m_data[(ry - y) * stride + (rx - x)] = depth(dx, dy);
					}
				}
				reply->set_dest(job_body->m_reply);
				//simulate failure !
				if (rand() % 100 < 10) return;
				global_router->send(reply);
			});
			break;
		}
		case select_reply:
		{
			//job reply, validate it's a current job not some old discarded one maybe
			auto reply_body = (Job_reply*)msg->begin();
			auto key = reply_body->m_key;
			auto itr = m_jobs_sent.find(key);
			if (itr == end(m_jobs_sent)) break;

			//use the reply data
			auto x = reply_body->m_x;
			auto y = reply_body->m_y;
			auto x1 = reply_body->m_x1;
			auto y1 = reply_body->m_y1;
			auto stride = x1 - x;
			for (auto ry = y; ry < y1; ++ry)
			{
				for (auto rx = x; rx < x1; ++rx)
				{
					uint32_t i = reply_body->m_data[(ry - y) * stride + (rx - x)];
					uint32_t col = argb_black;
					if (i != 255)
					{
						col = col + (i << 16) + ((i & 0x7f) << 9)+ ((i & 0x3f) << 2);
					}
					canvas->m_col = col;
					canvas->plot(rx, ry);
				}
			}
			m_dirty = true;

			//remove completed job, maybe send off another
			m_jobs_sent.erase(itr);
			dispatch_job();
			break;
		}
		case select_timer:
		{
			//restart timer, refresh workers
			Kernel_Service::timed_mail(m_select[select_timer], std::chrono::milliseconds(1000), 0);
			refresh_workers();

			//update display
			if (m_dirty)
			{
				m_dirty = false;
				canvas->swap();
			}

			//restart any jobs ?
			auto now = std::chrono::high_resolution_clock::now();
			auto cnt = 0;
			for (auto itr = begin(m_jobs_sent); itr != end(m_jobs_sent);)
			{
				auto job = itr->second;
				auto job_body = (Job*)job->begin();
				auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - job_body->m_time);
				if (age.count() > 1000)
				{
					itr = m_jobs_sent.erase(itr);
					m_jobs_ready.push_front(job);
					cnt++;
				}
				else ++itr;
			}
			for (auto i = 0; i < cnt; ++i) dispatch_job();
			break;
		}
		default:
		{
			//must be select_main
			//dispatch to widgets
			window->event(msg);
		}
		}
	}

	global_router->forget(m_entry);
	free_select(m_select);
}

uint8_t Mandelbrot_App::depth(double x0, double y0) const
{
	double xc = 0;
	double yc = 0;
	double x2 = 0;
	double y2 = 0;
	uint8_t i;
	for (i = 0; i != 255 && (x2 + y2) < 4.0; ++i)
	{
		yc = xc * yc * 2.0 + y0;
		xc = x2 - y2 + x0;
		x2 = xc * xc;
		y2 = yc * yc;
	}
	return i;
}

void Mandelbrot_App::dispatch_job()
{
	if (m_jobs_ready.empty()) return;
	auto job = m_jobs_ready.front();
	auto job_body = (Job*)job->begin();
	m_jobs_ready.pop_front();
	m_jobs_sent[m_key] = job;
	job_body->m_reply = m_select[select_reply];
	job_body->m_time = std::chrono::high_resolution_clock::now();
	job_body->m_key = m_key++;
	job->set_dest(m_workers[rand() % m_workers.size()]);
	global_router->send(job);
}

void Mandelbrot_App::refresh_workers()
{
	auto entries = global_router->enquire("mandel_worker");
	if (entries != m_entries)
	{
		m_entries = entries;
		m_workers.clear();
		for (auto &e : entries)
		{
			auto fields = split_string(e, ",");
			m_workers.push_back(Net_ID::from_string(fields[1]));
		}
	}
}
