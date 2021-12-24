#ifndef ROUTER_H
#define ROUTER_H

#include "directory.h"
#include "../links/link.h"
#include <thread>
#include <list>

//router class, holds registered peer links and routes messages
struct Route
{
	//increments on each routing ping
	unsigned int m_session = 0;
	//distance from the origin
	unsigned int m_hops = -1;
	//time this ping arrived
	std::chrono::high_resolution_clock::time_point m_time;
	//peers this ping has come via
	std::set<Dev_ID> m_vias;
};

//message que item
struct Que_Item
{
	//creation time
	std::chrono::high_resolution_clock::time_point m_time;
	//message
	std::shared_ptr<Msg> m_msg;
};

//the router is allocated a unique device id on creation and coordinates the routing
//and delivery of all messages
class Router
{
public:
	//constructed with some crypto true random Dev_ID !!!
	Router()
		: m_device_id(Dev_ID::alloc())
		, m_directory_manager(*this)
	{
		//start directory manager
		m_directory_manager.m_running = true;
		m_directory_thread = std::thread(&Directory_Manager::run, &m_directory_manager);
	}
	~Router()
	{
		//stop directory manager
		m_directory_manager.stop_thread();
		m_directory_thread.join();
	}
	//message and parcel sending
	void send(std::shared_ptr<Msg> &msg);
	auto _alloc_src()
	{
		auto src = Net_ID(m_device_id, m_next_parcel_id);
		m_next_parcel_id.m_id++;
		return src;
	}
	auto alloc_src()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return _alloc_src();
	}
	//get router device id
	auto const &get_dev_id() const { return m_device_id; }
	//mailbox alloc, free and validation
	auto alloc() { return Net_ID(m_device_id, m_mailbox_manager.alloc()); }
	void free(Net_ID id) { m_mailbox_manager.free(id.m_mailbox_id); }
	auto *validate(Net_ID id) { return m_mailbox_manager.validate(id.m_mailbox_id); }
	//directory management
	auto declare(const Net_ID &id, const std::string &service, const std::string &params)
		{ return m_directory_manager.declare(id, service, params); }
	auto forget(const std::string &entry)
		{ return m_directory_manager.forget(entry); }
	auto enquire(const std::string &prefix)
		{ return m_directory_manager.enquire(prefix); }
	auto enquire(const Dev_ID &dev_id, const std::string &prefix)
		{ return m_directory_manager.enquire(dev_id, prefix); }
	auto update_dir(const std::string &body) { return m_directory_manager.update(body); }
	//service broadcast helper
	void broadcast(std::vector<std::string> &services, std::shared_ptr<std::string> &body, const Net_ID &id = {{0}, 0});
	//registered peer links
	void add_link(Link *link, Dev_ID &id);
	void sub_link(Link *link);
	std::vector<Dev_ID> get_peers();
	//routing management
	std::shared_ptr<Msg> get_next_msg(const Dev_ID &dest, std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
	bool update_route(const std::string &body);
	void purge();
private:
	std::mutex m_mutex;
	std::thread m_directory_thread;
	std::condition_variable m_cv;
	const Dev_ID m_device_id;
	Mailbox_ID m_next_parcel_id;
	Mbox_Manager m_mailbox_manager;
	Directory_Manager m_directory_manager;
	std::map<Net_ID, std::pair<unsigned int, Que_Item>> m_parcels;
	std::list<Que_Item> m_outgoing_msg_que;
	std::map<Link*, Dev_ID> m_links;
	std::map<Dev_ID, Route> m_routes;
};

#endif
