#include "ip_link.h"
#include "../mail/router.h"
#include <iostream>
#include <cstring>

///////////
//utilities
///////////

uint32_t jenkins_hash(uint8_t *key, size_t len);
void obfuscate(uint8_t *key, size_t len);

////////////
//IP Manager
////////////

void IP_Link_Manager::run()
{
	//create a server or client or do nothing yet, the hub runs a server,
	//applications usually start up and immediately dial the local hub
	if (m_ip_addr == "server")
	{
		//we are going to be a server
		asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), IP_LINK_PORT);
		m_acceptor.open(endpoint.protocol());
		m_acceptor.bind(endpoint);
		m_acceptor.listen();
		accept();
	}
	else if (m_ip_addr != "")
	{
		//we are going to be a client
		dial(m_ip_addr);
	}

	while (m_running)
	{
		//got dialing to do ?
		if (!m_dial_que.empty())
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			for (auto &addr : m_dial_que) connect(addr);
			m_dial_que.clear();
		}

		//asio run loop/sleep
		auto start = std::chrono::high_resolution_clock::now();
		m_io_context.run_for(std::chrono::milliseconds(IP_LINK_MANAGER_POLLING_RATE));
		auto finish = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed = finish - start;
		if (elapsed.count() < IP_LINK_MANAGER_POLLING_RATE)
		{
			unsigned int sleep = IP_LINK_MANAGER_POLLING_RATE - elapsed.count();
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
		}

		//purge any dead/closed links
		m_links.erase(std::remove_if(begin(m_links), end(m_links), [&] (auto &link)
		{
			if (link->m_running) return false;
			std::cout << "ip_link: purged" << std::endl;
			link->stop_threads();
			link->join_threads();
			return true;
		}), end(m_links));
	}

	//close any links
	for (auto &link : m_links) link->stop_threads();
	for (auto &link : m_links) link->join_threads();
	m_links.clear();
}

void IP_Link_Manager::stop_thread()
{
	if (!m_running) return;
	Link_Manager::stop_thread();
	m_io_context.stop();
}

void IP_Link_Manager::accept()
{
	std::cout << "accept: waiting..." << std::endl;
	auto socket = std::make_shared<asio::ip::tcp::socket>(m_io_context);
	m_acceptor.async_accept(*socket, [socket, this] (const asio::error_code ec)
	{
		if (!ec)
		{
			std::cout << "accept: connected" << std::endl;
			m_links.emplace_back(std::make_unique<IP_Link>(m_router, socket));
			m_links.back()->start_threads();
		}
		else
		{
			std::cout << "accept: error, " << ec.message() << std::endl;
		}
		if (m_running) accept();
	});
}

void IP_Link_Manager::connect(const std::string &addr)
{
	asio::error_code ec;
	asio::ip::address ip_address = asio::ip::make_address(addr, ec);
	if (!ec)
	{
		std::cout << "connect: waiting..." << std::endl;
		asio::ip::tcp::endpoint ep(ip_address, IP_LINK_PORT);
		auto socket = std::make_shared<asio::ip::tcp::socket>(m_io_context);
		socket->async_connect(ep, [socket, this] (const asio::error_code ec)
		{
			if (!ec)
			{
				std::cout << "connect: connected" << std::endl;
				m_links.emplace_back(std::make_unique<IP_Link>(m_router, socket));
				m_links.back()->start_threads();
			}
			else
			{
				std::cout << "connect: error, " << ec.message() << std::endl;
			}
		});
	}
}

void IP_Link_Manager::dial(const std::string &addr)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_dial_que.emplace_back(addr);
}

/////////
//IP link
/////////

//send msg header and body down the link
bool IP_Link::send(const std::shared_ptr<Msg> &msg)
{
	//pack msg into send buffer, calculate the hash and obfuscate
	uint32_t len = offsetof(Link_Buf, m_msg_body) + msg->m_header.m_frag_length;
	memcpy(&m_send_buf.m_dev_id, &m_router.get_dev_id(), sizeof(Dev_ID));
	memcpy((uint8_t*)&m_send_buf.m_msg_header, &msg->m_header, sizeof(Msg_Header));
	memcpy(&m_send_buf.m_msg_body, msg->begin() + msg->m_header.m_data_offset, msg->m_header.m_frag_length);
	m_send_buf.m_hash = jenkins_hash((uint8_t*)&m_send_buf.m_dev_id, offsetof(Link_Buf, m_msg_body) - offsetof(Link_Buf, m_dev_id) + msg->m_header.m_frag_length);
	obfuscate((uint8_t*)&m_send_buf, len);

	//write the buffer to the socket
	try
	{
		asio::write(*m_socket, asio::buffer((uint8_t*)&len, sizeof(len)));
		asio::write(*m_socket, asio::buffer((uint8_t*)&m_send_buf, len));
	}
	catch(const std::exception& e)
	{
		m_running = false;
		return false;
	}
	return true;
}

std::shared_ptr<Msg> IP_Link::receive()
{
	//read the buffer from the socket
	uint32_t len = 0;
	try
	{
		asio::read(*m_socket, asio::buffer((uint8_t*)&len, sizeof(len)));
		asio::read(*m_socket, asio::buffer((uint8_t*)&m_receive_buf, len));
	}
	catch(const std::exception& e)
	{
		m_running = false;
		return nullptr;
	}

	//un-obfuscate and calculate the hash
	obfuscate((uint8_t*)&m_receive_buf, len);
	auto hash = jenkins_hash((uint8_t*)&m_receive_buf.m_dev_id, len - offsetof(Link_Buf, m_dev_id));
	if (hash != m_receive_buf.m_hash)
	{
		//error with crc hash !!!
		std::cerr << "ip_link: crc error !" << std::endl;
		return nullptr;
	}

	//unpack msg from receive buffer
	auto msg = std::make_shared<Msg>(m_receive_buf.m_msg_header, m_receive_buf.m_msg_body);

	//refresh who we are connected to in a unplug/plug scenario.
	//if the peer device id changes we need to swap the link on the router !
	//the software equivelent of pulling the lead out and plugging another one in.
	if (m_receive_buf.m_dev_id != m_remote_dev_id)
	{
		m_remote_dev_id = m_receive_buf.m_dev_id;
		m_router.sub_link(this);
		m_router.add_link(this, m_remote_dev_id);
	}
	return msg;
}
