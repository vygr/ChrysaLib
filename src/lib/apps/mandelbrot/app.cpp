#include "app.h"
#include "../../gui/ui.h"
#include <cstring>

std::string to_utf8(uint32_t c);
uint32_t from_utf8(uint8_t **data);

void Mandelbrot_App::run()
{
	enum
	{
		event_close,
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
					ui_connect(event_close)
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
				auto job_body = (Mandelbrot_Job*)msg_ref->begin();
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
				auto reply = std::make_shared<Msg>(sizeof(Mandelbrot_Job_reply)
					+ (stride * (y1 - y) * sizeof(uint8_t)));
				auto reply_body = (Mandelbrot_Job_reply*)reply->begin();
				//must return job header
				memcpy(&reply_body->m_worker, &job_body->m_worker, sizeof(Farm::Job));
				//now our specific data
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
			if (!m_farm->validate_job(msg)) break;
			auto reply_body = (Mandelbrot_Job_reply*)msg->begin();

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
			m_farm->complete_job(msg);
			break;
		}
		case select_timer:
		{
			//restart timer
			Kernel_Service::timed_mail(m_select[select_timer], std::chrono::milliseconds(UPDATE_TIMEOUT), 0);

			//adjust workforce and jobs
			m_farm->refresh();

			//update display
			if (m_dirty)
			{
				m_dirty = false;
				canvas->swap();
			}
			break;
		}
		case select_main:
		{
			//must be select_main
			auto msg_body = (View::Event*)msg->begin();
			switch (msg_body->m_target_id)
			{
			case event_close:
			{
				Kernel_Service::stop_task(shared_from_this());
				break;
			}
			default:
			{
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
				break;
			}
			}
			break;
		}
		}
	}

	//tidy up
	window->hide();
	global_router->forget(m_entry);
	free_select(m_select);
}

uint8_t Mandelbrot_App::depth(double x0, double y0) const
{
	double xc = 0;
	double yc = 0;
	double x2 = 0;
	double y2 = 0;
	uint32_t i;
	for (i = 0; i != 255 && (x2 + y2) < 4.0; ++i)
	{
		yc = xc * yc * 2.0 + y0;
		xc = x2 - y2 + x0;
		x2 = xc * xc;
		y2 = yc * yc;
	}
	return i;
}

void Mandelbrot_App::reset()
{
	//new reply mailbox !
	global_router->free(m_select[select_reply]);
	m_select[select_reply] = global_router->alloc();

	//create farm, will kill old one
	m_farm = std::make_unique<Farm>("mandel_worker",
		JOB_LIMIT,
		std::chrono::milliseconds(JOB_TIMEOUT),
		[&] (auto &worker, std::shared_ptr<Msg> job)
		{
			//fill in what I need before dispatch
			auto job_body = (Mandelbrot_Job*)job->begin();
			job_body->m_reply = m_select[select_reply];
		});

	//fill farm job que
	for (auto y = 0; y < CANVAS_HEIGHT * CANVAS_SCALE; ++y)
	{
		auto job = std::make_shared<Msg>(sizeof(Mandelbrot_Job));
		auto job_body = (Mandelbrot_Job*)job->begin();
		job_body->m_x = 0;
		job_body->m_y = y;
		job_body->m_x1 = CANVAS_WIDTH * CANVAS_SCALE;
		job_body->m_y1 = y + 1;
		job_body->m_cw = CANVAS_WIDTH * CANVAS_SCALE;
		job_body->m_ch = CANVAS_HEIGHT * CANVAS_SCALE;
		job_body->m_cx = m_cx;
		job_body->m_cy = m_cy;
		job_body->m_z = m_zoom;
		m_farm->add_job(job);
	}

	//get to work !
	m_farm->assign_work();
}
