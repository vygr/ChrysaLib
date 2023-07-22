#include "app.h"
#include "../../gui/ui.h"
#include <cstring>

std::string to_utf8(uint32_t c);
uint32_t from_utf8(uint8_t **data);

void Raymarch_App::run()
{
	enum
	{
		event_close, //must be first !
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
					{"text", "Raymarch"}}))
				ui_end
			ui_end
			ui_canvas(canvas, CANVAS_WIDTH, CANVAS_HEIGHT, CANVAS_SCALE, ({}))
			ui_end
		ui_end
	ui_end

	//clear canvas to black
	canvas->set_col(argb_black);
	canvas->fill();
	canvas->swap(0);

	//add to my GUI screen
	window->change(locate(window->pref_size()));
	add_front(window);

	//select and init workers
	m_select = alloc_select(select_size);
	m_entry = global_router->declare(m_select[select_worker], "raymarch_worker", "Raymarch v0.01");
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
				auto job_body = (Raymarch_Job*)msg_ref->begin();
				auto over_sample = job_body->m_over_sample;
				auto x = job_body->m_x;
				auto y = job_body->m_y;
				auto x1 = job_body->m_x1;
				auto y1 = job_body->m_y1;
				auto cw = job_body->m_cw;
				auto ch = job_body->m_ch;
				auto stride = (x1 - x);
				auto reply = std::make_shared<Msg>(sizeof(Raymarch_Job_reply) + stride * (y1 - y) * sizeof(uint32_t));
				auto reply_body = (Raymarch_Job_reply*)reply->begin();
				//must return job header
				memcpy(&reply_body->m_worker, &job_body->m_worker, sizeof(Farm::Job));
				//now our specific data
				reply_body->m_x = x;
				reply_body->m_y = y;
				reply_body->m_x1 = x1;
				reply_body->m_y1 = y1;
				render(reply_body, over_sample, x, y, x1, y1, cw, ch);
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
			auto reply_body = (Raymarch_Job_reply*)msg->begin();

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
					uint32_t col = reply_body->m_data[(ry - y) * stride + (rx - x)];
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
				canvas->swap(0);
			}
			break;
		}
		case select_main:
		{
			//must be select_main
			auto msg_body = (View::Event*)msg->begin();
			switch (msg_body->m_evt)
			{
			case event_close:
			{
				Kernel_Service::stop_task(shared_from_this());
				break;
			}
			default:
			{
				if (msg_body->m_evt == canvas->get_id()
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
						canvas->set_col(argb_black);
						canvas->fill();
						canvas->swap(0);
						reset();
					}
					else if (mouse_body->m_buttons == 3)
					{
						//zoom out
						auto size = canvas->get_size();
						auto rx = mouse_body->m_rx - (size.m_w - CANVAS_WIDTH) / 2;
						auto ry = mouse_body->m_ry - (size.m_h - CANVAS_HEIGHT) / 2;
						canvas->set_col(argb_black);
						canvas->fill();
						canvas->swap(0);
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
	sub(window);
	global_router->forget(m_entry);
	free_select(m_select);
}

void Raymarch_App::reset()
{
	//new reply mailbox !
	global_router->free(m_select[select_reply]);
	m_select[select_reply] = global_router->alloc();

	//create farm, will kill old one
	m_farm = std::make_unique<Farm>("raymarch_worker",
		JOB_LIMIT,
		std::chrono::milliseconds(JOB_TIMEOUT),
		[&] (auto &worker, std::shared_ptr<Msg> job)
		{
			//fill in what I need before dispatch
			auto job_body = (Raymarch_Job*)job->begin();
			job_body->m_reply = m_select[select_reply];
		});

	//fill farm job que
	for (auto y = 0; y < CANVAS_HEIGHT * CANVAS_SCALE; ++y)
	{
		auto job = std::make_shared<Msg>(sizeof(Raymarch_Job));
		auto job_body = (Raymarch_Job*)job->begin();
		job_body->m_over_sample = OVER_SAMPLE;
		job_body->m_x = 0;
		job_body->m_y = y;
		job_body->m_x1 = CANVAS_WIDTH * CANVAS_SCALE;
		job_body->m_y1 = y + 1;
		job_body->m_cw = CANVAS_WIDTH * CANVAS_SCALE;
		job_body->m_ch = CANVAS_HEIGHT * CANVAS_SCALE;
		m_farm->add_job(job);
	}

	//get to work !
	m_farm->assign_work();
}
