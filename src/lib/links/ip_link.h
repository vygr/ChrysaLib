#ifndef IP_LINK_H
#define IP_LINK_H

#include "link.h"
#include <asio.hpp>

class IP_Link : public Link
{
public:
	IP_Link(Router &router, std::shared_ptr<asio::ip::tcp::socket> socket)
		: Link(router)
		, m_socket(socket)
	{}
	std::shared_ptr<asio::ip::tcp::socket> m_socket;
protected:
	virtual bool send(const std::shared_ptr<Msg> &msg) override;
	virtual std::shared_ptr<Msg> receive() override;
};

class IP_Link_Manager : public Link_Manager
{
public:
	IP_Link_Manager(Router &router, const std::string &ip_addr)
		: Link_Manager(router)
		, m_ip_addr(ip_addr)
		, m_io_context()
		, m_acceptor(m_io_context)
	{}
	void start_thread() override
	{
		m_running = true;
		m_thread = std::thread(&IP_Link_Manager::run, this);
	}
	void stop_thread() override;
	void join_thread() override { if (m_thread.joinable()) m_thread.join(); }
	void dial(const std::string &addr);
private:
	void run() override;
	void accept();
	void connect(const std::string &addr);
	std::mutex m_mutex;
	std::thread m_thread;
	std::vector<std::unique_ptr<IP_Link>> m_links;
	std::vector<std::string> m_dial_que;
	std::string m_ip_addr;
	asio::io_context m_io_context;
	asio::ip::tcp::acceptor m_acceptor;
};

#endif
