#ifndef LINK_H
#define LINK_H

#include "../../host/pii.h"
#include "../mail/msg.h"
#include "../settings.h"
#include <thread>

class Router;

//links are point to point packet transfer processes.
//they scan the outgoing msg que for packets that go to their destination.
//a link sends the header and body to the destination.
//included is the hash of the link buffer for error detection plus
//the Node_ID of the peer who sent it.
struct Link_Buf
{
	uint32_t m_hash = 0;
	Node_ID m_dev_id;
	Msg_Header m_msg_header;
	uint8_t m_msg_body[lk_data_size] = {0};
};

class Link
{
public:
	Link() {}
	virtual ~Link() {}
	virtual void start_threads()
	{
		m_running = true;
		m_thread_send = std::thread(&Link::run_send, this);
		m_thread_receive = std::thread(&Link::run_receive, this);
	}
	virtual void join_threads()
	{
		if (m_thread_send.joinable()) m_thread_send.join();
		if (m_thread_receive.joinable()) m_thread_receive.join();
	}
	virtual void stop_threads() { m_running = false; }
	virtual void run_send();
	virtual void run_receive();
	bool m_running = false;
protected:
	//send/receive, override these for specific sub class
	virtual bool send(const std::shared_ptr<Msg> &msg) = 0;
	virtual std::shared_ptr<Msg> receive() = 0;
	std::thread m_thread_send;
	std::thread m_thread_receive;
	Node_ID m_remote_dev_id;
	Link_Buf m_send_buf;
	Link_Buf m_receive_buf;
};

//link managers are responsible for the discovery and management of a link subclass.
//as links are discovered or removed it adds and removes them from the router.
class Link_Manager
{
public:
	Link_Manager() {}
	virtual ~Link_Manager() {}
	virtual void start_thread() = 0;
	virtual void stop_thread() { m_running = false; }
	virtual void join_thread() = 0;
	bool m_running = false;
protected:
	//thread executes this run method
	virtual void run() = 0;
};

#endif
