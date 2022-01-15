#include "farm.h"
#include "../mail/router.h"
#include <chrono>
#include <algorithm>

extern std::unique_ptr<Router> global_router;

void Farm::add_job(std::shared_ptr<Msg> job)
{
	m_jobs_ready.emplace_back(job);
}

bool Farm::validate_job(std::shared_ptr<Msg> job)
{
	auto reply_body = (Job*)job->begin();
	auto worker = reply_body->m_worker;
	auto key = reply_body->m_key;
	auto itr = m_jobs_assigned.find(worker);
	if (itr == end(m_jobs_assigned)) return false;
	auto &tickets = itr->second;
	auto titr = std::find_if(begin(tickets), end(tickets), [&] (auto &ticket)
	{
		auto job_body = (Job*)ticket.m_job->begin();
		return key == job_body->m_key;
	});
	return titr != end(tickets);
}

std::vector<Net_ID> Farm::census()
{
	auto census = std::vector<Net_ID>{};
	auto entries = global_router->enquire(m_service_prefix);
	for (auto &e : entries)
	{
		auto fields = split_string(e, ",");
		census.push_back(Net_ID::from_string(fields[1]));
	}
	return census;
}

void Farm::joiners(const std::vector<Net_ID> &census)
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

void Farm::leavers(const std::vector<Net_ID> &census)
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

void Farm::add_worker(const Net_ID &worker)
{
	if (m_jobs_ready.empty()) return;
	dispatch(worker, m_jobs_ready.front());
	m_jobs_ready.pop_front();
}

void Farm::sub_worker(const Net_ID &worker)
{
	auto itr = m_jobs_assigned.find(worker);
	if (itr != end(m_jobs_assigned))
	{
		auto &tickets = itr->second;
		std::for_each(begin(tickets), end(tickets), [&] (auto &t)
		{
			m_jobs_ready.emplace_back(std::move(t.m_job));
		});
		m_jobs_assigned.erase(itr);
	}
}

void Farm::dispatch(const Net_ID &worker, std::shared_ptr<Msg> job)
{
	auto now = std::chrono::high_resolution_clock::now();
	m_jobs_assigned[worker].push_back(ticket{job, now});
	auto job_body = (Job*)job->begin();
	job_body->m_worker = worker;
	job_body->m_key = m_job_key++;
	m_dispatch(worker, job);
	job->set_dest(worker);
	global_router->send(job);
}

void Farm::restart()
{
	auto now = std::chrono::high_resolution_clock::now();
	for (auto &itr : m_jobs_assigned)
	{
		auto &tickets = itr.second;
		for (auto titr = begin(tickets); titr != end(tickets);)
		{
			auto then = titr->m_time;
			auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - then);
			if (age > m_timeout)
			{
				m_jobs_ready.emplace_front(titr->m_job);
				titr = tickets.erase(titr);
			}
			else ++titr;
		}
	}
}

void Farm::assign_work()
{
	if (m_jobs_ready.empty()) return;
	for (auto itr = begin(m_jobs_ready); itr != end(m_jobs_ready);)
	{
		const Net_ID *worker = nullptr;
		size_t cnt = 1000000;
		for (auto &entry : m_jobs_assigned)
		{
			auto c = entry.second.size();
			if (c < m_job_limit && c < cnt)
			{
				worker = &entry.first;
				cnt = entry.second.size();
			}
		}
		if (cnt == 1000000) return;		
		dispatch(*worker, *itr);
		itr = m_jobs_ready.erase(itr);
	}
}

void Farm::complete_job(std::shared_ptr<Msg> job)
{
	auto job_body = (Job*)job->begin();
	auto worker = job_body->m_worker;
	auto key = job_body->m_key;
	auto itr = m_jobs_assigned.find(worker);
	if (itr != end(m_jobs_assigned))
	{
		auto &tickets = itr->second;
		auto titr = std::find_if(begin(tickets), end(tickets), [&] (auto &ticket)
		{
			auto job_body = (Job*)ticket.m_job->begin();
			return key == job_body->m_key;
		});
		if (titr == end(tickets)) return;
		tickets.erase(titr);
		if (m_jobs_ready.empty()) return;
		dispatch(worker, m_jobs_ready.front());
		m_jobs_ready.pop_front();
	}
}

void Farm::refresh()
{
	auto workers = census();
	restart();
	leavers(workers);
	joiners(workers);
	assign_work();
}
