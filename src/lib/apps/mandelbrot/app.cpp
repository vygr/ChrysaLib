#include "app.h"
#include "../../gui/ui.h"
#include <assert.h>

std::string to_utf8(uint32_t c);
uint32_t from_utf8(uint8_t **data);

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

	//clear canvas to black
	canvas->set_col(argb_black);
	canvas->fill();
	canvas->swap();
	auto s = window->pref_size();
	window->change(120, 64, s.m_w, s.m_h);

	//add to my GUI screen
	add_front(window);

	//select and init workers
	m_select = alloc_select(select_size);
	m_entry = global_router->declare(m_select[select_worker], "mandel_worker", "Mandelbrot v0.01");
	reset();

	//event loop
	Kernel_Service::timed_mail(m_select[select_timer], std::chrono::milliseconds(1), 0);
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
				reply_body->m_worker = job_body->m_worker;
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
				//if (rand() % 100 < 5) return;
				global_router->send(reply);
			});
			break;
		}
		case select_reply:
		{
			//job reply, validate it's a current job not some old discarded one
			auto reply_body = (Job_reply*)msg->begin();
			auto worker = reply_body->m_worker;
			auto itr = m_jobs_assigned.find(worker);
			if (itr == end(m_jobs_assigned)) break;

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
			complete(worker, reply_body->m_key);
			break;
		}
		case select_timer:
		{
			//restart timer
			Kernel_Service::timed_mail(m_select[select_timer], std::chrono::milliseconds(JOB_TIMEOUT), 0);

			//adjust workforce and jobs
			restart();
			auto workers = census();
			leavers(workers);
			joiners(workers);
			slackers();

			//update display
			if (m_dirty)
			{
				m_dirty = false;
				canvas->swap();
			}
			break;
		}
		default:
		{
			//must be select_main
			auto msg_body = (View::Event*)msg->begin();
			if (msg_body->m_target_id == canvas->get_id()
				&& msg_body->m_type == ev_type_mouse)
			{
				//canvas mouse event
				auto mouse_body = (View::Event_mouse*)msg_body;
				if (mouse_body->m_buttons == 1)
				{
					//zoom in
					auto size = canvas->get_size();
					auto rx = mouse_body->m_rx - (size.m_w - CANVAS_WIDTH) / 2;
					auto ry = mouse_body->m_ry - (size.m_h - CANVAS_HEIGHT) / 2;
					m_cx += (((((double)rx - (CANVAS_WIDTH / 2.0)) * 2.0) / CANVAS_WIDTH) * m_zoom);
					m_cy += (((((double)ry - (CANVAS_HEIGHT / 2.0)) * 2.0) / CANVAS_HEIGHT) * m_zoom);
					m_zoom *= 0.5;
					reset();
				}
				else if (mouse_body->m_buttons == 3)
				{
					//zoom out
					auto size = canvas->get_size();
					auto rx = mouse_body->m_rx - (size.m_w - CANVAS_WIDTH) / 2;
					auto ry = mouse_body->m_ry - (size.m_h - CANVAS_HEIGHT) / 2;
					m_cx += (((((double)rx - (CANVAS_WIDTH / 2.0)) * 2.0) / CANVAS_WIDTH) * m_zoom);
					m_cy += (((((double)ry - (CANVAS_HEIGHT / 2.0)) * 2.0) / CANVAS_HEIGHT) * m_zoom);
					m_zoom *= 2.0;
					reset();
				}
			}
			else
			{
				//dispatch to widgets
				window->event(msg);
			}
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

std::vector<Net_ID> Mandelbrot_App::census()
{
	auto census = std::vector<Net_ID>{};
	auto entries = global_router->enquire("mandel_worker");
	for (auto &e : entries)
	{
		auto fields = split_string(e, ",");
		census.push_back(Net_ID::from_string(fields[1]));
	}
	return census;
}

void Mandelbrot_App::joiners(const std::vector<Net_ID> &census)
{
	std::copy_if(begin(census), end(census), std::back_inserter(m_workers), [&] (auto &worker)
	{
		auto itr = std::find(begin(m_workers), end(m_workers), worker);
		if (itr == end(m_workers))
		{
			add_worker(worker);
			return true;
		}
		return false;
	});
}

void Mandelbrot_App::leavers(const std::vector<Net_ID> &census)
{
	m_workers.erase(std::remove_if(begin(m_workers), end(m_workers), [&] (auto &worker)
	{
		auto itr = std::find(begin(census), end(census), worker);
		if (itr == end(census))
		{
			sub_worker(worker);
			return true;
		}
		return false;
	}), end(m_workers));
}

void Mandelbrot_App::add_worker(const Net_ID &worker)
{
	if (m_jobs_ready.empty()) return;
	auto job = m_jobs_ready.front();
	m_jobs_ready.pop_front();
	dispatch(job, worker);
}

void Mandelbrot_App::sub_worker(const Net_ID &worker)
{
	auto itr = m_jobs_assigned.find(worker);
	if (itr != end(m_jobs_assigned))
	{
		for (auto &ticket : itr->second) m_jobs_ready.push_back(ticket.m_job);
		m_jobs_assigned.erase(itr);
	}
}

void Mandelbrot_App::dispatch(std::shared_ptr<Msg> job, const Net_ID &worker)
{
	auto now = std::chrono::high_resolution_clock::now();
	m_jobs_assigned[worker].push_back(ticket{job, now});
	auto job_body = (Job*)job->begin();
	job_body->m_reply = m_select[select_reply];
	job_body->m_worker = worker;
	job->set_dest(worker);
	global_router->send(job);
}

void Mandelbrot_App::restart()
{
	auto now = std::chrono::high_resolution_clock::now();
	for (auto &itr : m_jobs_assigned)
	{
		auto &tickets = itr.second;
		for (auto itr = begin(tickets); itr != end(tickets);)
		{
			auto then = itr->m_time;
			auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - then);
			if (age.count() > JOB_TIMEOUT)
			{
				auto job = itr->m_job;
				m_jobs_ready.push_front(job);
				itr = tickets.erase(itr);
			}
			else ++itr;
		}
	}
}

void Mandelbrot_App::slackers()
{
	if (m_jobs_ready.empty()) return;
	for (auto itr = begin(m_jobs_ready); itr != end(m_jobs_ready);)
	{
		const Net_ID *worker = nullptr;
		size_t cnt = 1000000;
		for (auto &entry : m_jobs_assigned)
		{
			auto c = entry.second.size();
			if (c < JOB_LIMIT && c < cnt)
			{
				worker = &entry.first;
				cnt = entry.second.size();
			}
		}
		if (cnt == 1000000) return;		
		auto job = *itr;
		dispatch(job, *worker);
		itr = m_jobs_ready.erase(itr);
	}
}

void Mandelbrot_App::complete(const Net_ID &worker, uint32_t key)
{
	auto itr = m_jobs_assigned.find(worker);
	if (itr != end(m_jobs_assigned))
	{
		auto &tickets = itr->second;
		auto itr = std::find_if(begin(tickets), end(tickets), [&] (auto &ticket)
		{
			auto job = ticket.m_job;
			auto job_body = (Job*)job->begin();
			return key == job_body->m_key;
		});
		if (itr == end(tickets)) return;
		tickets.erase(itr);
		if (m_jobs_ready.empty()) return;
		auto job = m_jobs_ready.front();
		m_jobs_ready.pop_front();
		dispatch(job, worker);
	}
}

void Mandelbrot_App::reset()
{
	//new reply mailbox !
	global_router->free(m_select[select_reply]);
	m_select[select_reply] = global_router->alloc();

	//clear out old assignments
	for (auto &work : m_jobs_assigned) work.second.clear();

	//fill job que
	auto key = 0;
	m_jobs_ready.clear();
	for (auto y = 0; y < CANVAS_HEIGHT * CANVAS_SCALE; ++y)
	{
		m_jobs_ready.emplace_back(std::make_shared<Msg>(sizeof(Job)));
		auto job_body = (Job*)m_jobs_ready.back()->begin();
		job_body->m_key = key++;
		job_body->m_x = 0;
		job_body->m_y = y;
		job_body->m_x1 = CANVAS_WIDTH * CANVAS_SCALE;
		job_body->m_y1 = y + 1;
		job_body->m_cw = CANVAS_WIDTH * CANVAS_SCALE;
		job_body->m_ch = CANVAS_HEIGHT * CANVAS_SCALE;
		job_body->m_cx = m_cx;
		job_body->m_cy = m_cy;
		job_body->m_z = m_zoom;
	}

	//get to work !
	slackers();
}
