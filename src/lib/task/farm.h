#ifndef FARM_H
#define FARM_H

#include "../mail/msg.h"
#include <vector>
#include <list>
#include <map>

class Farm
{
public:
	struct Job
	{
		Net_ID m_worker;
		uint32_t m_key;
	};
	Farm(const std::string &service_prefix,
		uint32_t job_limit,
		std::chrono::milliseconds timeout,
		std::function<void(const Net_ID &, std::shared_ptr<Msg> job)> dispatch)
		: m_service_prefix(service_prefix)
		, m_job_limit(job_limit)
		, m_timeout(timeout)
		, m_dispatch(dispatch)
	{}
	void add_job(std::shared_ptr<Msg> job);
	bool validate_job(std::shared_ptr<Msg> job);
	void complete_job(std::shared_ptr<Msg> job);
	void assign_work();
	void refresh();
private:
	struct ticket
	{
		std::shared_ptr<Msg> m_job;
		std::chrono::high_resolution_clock::time_point m_time;
	};
	std::vector<Net_ID> census();
	void joiners(const std::vector<Net_ID> &census);
	void leavers(const std::vector<Net_ID> &census);
	void add_worker(const Net_ID &worker);
	void sub_worker(const Net_ID &worker);
	void dispatch(const Net_ID &worker, std::shared_ptr<Msg> job);
	void restart();
	std::vector<Net_ID> m_workers;
	std::list<std::shared_ptr<Msg>> m_jobs_ready;
	std::map<Net_ID, std::list<ticket>> m_jobs_assigned;
	const std::string m_service_prefix;
	const std::function<void(const Net_ID &worker, std::shared_ptr<Msg> job)> m_dispatch;
	const std::chrono::milliseconds m_timeout;
	const uint32_t m_job_limit;
	uint32_t m_job_key = 0;
};

#endif
