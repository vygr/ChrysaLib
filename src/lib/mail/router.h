#ifndef ROUTER_H
#define ROUTER_H

#include "../links/link.h"
#include "net.h"
#include <thread>
#include <list>
#include <set>

//router class, holds registered peer links and routes messages.

//directory management.
//maintain a set of services for each node, on each node.
//a service entry is of the format: "service_name,dev_id:mbox_id,..."

//message mailbox management and validation.
//manage the allocation and freeing of local mailboxes and the ability to
//wait on or test the availability of messages.

//directory entry record
struct Directory
{
	//session number will increment each time it changes
	uint32_t m_session = 0;
	//the last time our device saw it change
	std::chrono::high_resolution_clock::time_point m_time_modified;
	//the set of all services on that device
	std::set<std::string> m_services;
};

struct Route
{
	//increments on each routing ping
	uint32_t m_session = 0;
	//distance from the origin
	uint32_t m_hops = -1;
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
	{
		//start directory manager
		m_running = true;
		m_thread = std::thread(&Router::run, this);
	}
	~Router()
	{
		//stop directory manager
		stop_thread();
		m_thread.join();
	}
	void run();
	void stop_thread();
	//message and parcel sending
	void send(std::shared_ptr<Msg> &msg);
	//get router device id
	auto const &get_dev_id() const { return m_device_id; }
	//mailbox alloc, free and validation
	Net_ID alloc();
	void free(const Net_ID &id);
	Mbox<std::shared_ptr<Msg>> *validate(const Net_ID &id);
	//read, poll and select
	std::shared_ptr<Msg> read(const Net_ID &id);
	int32_t poll(const std::vector<Net_ID> &ids);
	int32_t select(const std::vector<Net_ID> &ids);
	//directory management
	std::string declare(const Net_ID &id, const std::string &service, const std::string &params);
	void forget(const std::string &entry);
	std::vector<std::string> enquire(const std::string &prefix);
	std::vector<std::string> enquire(const Dev_ID &dev_id, const std::string &prefix);
	bool update_dir(const std::string &body);
	//service broadcast helper
	void broadcast(const std::vector<std::string> &services, std::shared_ptr<std::string> &body, const Net_ID &id = {{0}, 0});
	//registered peer links
	void add_link(Link *link, const Dev_ID &id);
	void sub_link(Link *link);
	std::vector<Dev_ID> get_peers();
	//routing management
	std::shared_ptr<Msg> get_next_msg(const Dev_ID &dest, std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
	bool update_route(const std::string &body);
	bool m_running = false;
private:
	void purge_routes();
	void purge_dir();
	Mbox<std::shared_ptr<Msg>> *validate_no_lock(const Net_ID &id);
	Net_ID alloc_src_no_lock();
	Net_ID alloc_src();
	std::mutex m_mutex;
	std::thread m_thread;
	std::condition_variable m_cv;
	const Dev_ID m_device_id;
	Mailbox_ID m_next_parcel_id;
	std::map<Net_ID, std::pair<uint32_t, Que_Item>> m_parcels;
	std::list<Que_Item> m_outgoing_msg_que;
	std::map<Link*, Dev_ID> m_links;
	std::map<Dev_ID, Route> m_routes;
	std::map<Dev_ID, Directory> m_directory;
	Mbox<Router*> m_wake_mbox;
	Mailbox_ID m_next_mailbox_id;
	std::map<Mailbox_ID, Mbox<std::shared_ptr<Msg>>> m_mailboxes;
};

#endif
