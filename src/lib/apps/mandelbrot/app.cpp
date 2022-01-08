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
	enum
	{
		select_main,
		select_reply,
		select_worker,
		select_timer,
		select_size,
	};

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

	//select
	auto select = alloc_select(select_size);

	//job ques
	auto jobs_ready = std::list<std::shared_ptr<Msg>>{};
	auto jobs_sent = std::list<std::shared_ptr<Msg>>{};
	uint32_t key = 0;
	for (auto y = 0; y < CANVAS_HEIGHT * CANVAS_SCALE; ++y)
	{
		jobs_ready.emplace_back(std::make_shared<Msg>(sizeof(Job)));
		auto job_body = (Job*)jobs_ready.back()->begin();
		job_body->m_reply = select[select_reply];
		job_body->m_time = std::chrono::high_resolution_clock::now();
		job_body->m_key = key++;
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

	//send off 16 jobs, just for  now
	for (auto i = 0; i < 16; ++i)
	{
		if (jobs_ready.empty()) break;
		auto job = jobs_ready.front();
		jobs_ready.pop_front();
		jobs_sent.push_back(job);
		job->set_dest(select[select_worker]);
		global_router->send(job);
	}

	//event loop
	Kernel_Service::timed_mail(select[select_timer], std::chrono::milliseconds(1000), 0);
	while (m_running)
	{
		auto idx = global_router->select(select);
		auto msg = global_router->read(select[idx]);
		switch (idx)
		{
		case select_worker:
		{
			//job request, hive off to worker thread
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
				global_router->send(reply);
			});
			break;
		}
		case select_reply:
		{
			//job reply
			auto reply_body = (Job_reply*)msg->begin();
			auto rep_key = reply_body->m_key;
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

			//remove completed job
			auto itr = std::find_if(begin(jobs_sent), end(jobs_sent), [&] (const auto &job)
			{
				return ((Job*)job->begin())->m_key == rep_key;
			});
			if (itr != end(jobs_sent)) jobs_sent.erase(itr);

			//send off another job ?
			if (jobs_ready.empty()) break;
			auto job = jobs_ready.front();
			jobs_ready.pop_front();
			jobs_sent.push_back(job);
			job->set_dest(select[select_worker]);
			global_router->send(job);
			break;
		}
		case select_timer:
		{
			//update display
			if (m_dirty)
			{
				m_dirty = false;
				Kernel_Service::timed_mail(select[select_timer], std::chrono::milliseconds(1000), 0);
				canvas->swap();
			}

			//restart any jobs ?
			auto now = std::chrono::high_resolution_clock::now();
			for (auto &job : jobs_sent)
			{
				auto job_body = (Job*)job->begin();
				auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - job_body->m_time);
				if (age.count() > 1000)
				{
					job_body->m_time = now;
					global_router->send(job);
				}
			}
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
	free_select(select);
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
