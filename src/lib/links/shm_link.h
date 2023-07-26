#ifndef SHM_LINK_H
#define SHM_LINK_H

#include "../../host/pii.h"
#include "link.h"

class SHM_Link : public Link
{
public:
	SHM_Link(const std::string &name)
		: Link()
	{}
protected:
	virtual bool send(const std::shared_ptr<Msg> &msg) override;
	virtual std::shared_ptr<Msg> receive() override;
private:
};

class SHM_Link_Manager : public Link_Manager
{
public:
	SHM_Link_Manager()
		: Link_Manager()
	{}
	void start_thread() override
	{
		m_running = true;
		m_thread = std::thread(&SHM_Link_Manager::run, this);
	}
	void join_thread() override { if (m_thread.joinable()) m_thread.join(); }
private:
	void run() override;
	std::thread m_thread;
	std::vector<std::unique_ptr<SHM_Link>> m_links;
};

#endif
