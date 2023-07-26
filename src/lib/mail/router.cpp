#include "../../host/pii.h"
#include "router.h"
#include "../settings.h"
#include "../services/kernel_service.h"
#include <algorithm>
#include <cstring>

/////////////////////
// mailbox management
/////////////////////

Net_ID Router::alloc()
{
	//allocate a new net id and enter the associated mailbox into the validation map.
	//it gets the next mailbox id that does not already exist and which will not repeat
	//for a very long time.
	std::lock_guard<std::mutex> l(m_mutex);
	while (m_next_mailbox_id.m_id == MAX_ID
		|| m_mailboxes.find(m_next_mailbox_id) != end(m_mailboxes)) m_next_mailbox_id.m_id++;
	auto id = m_next_mailbox_id;
	m_mailboxes[id];
	m_next_mailbox_id.m_id++;
	return Net_ID(m_node_id, id);
}

void Router::free(const Net_ID &id)
{
	//free the mailbox associated with this net id
	std::lock_guard<std::mutex> l(m_mutex);
	auto itr = m_mailboxes.find(id.m_mailbox_id);
	if (itr != end(m_mailboxes)) m_mailboxes.erase(itr);
}

Mbox<std::shared_ptr<Msg>> *Router::validate_no_lock(const Net_ID &id)
{
	//validate that this net id has a mailbox associated with it.
	//return the mailbox if so, otherwise nullptr.
	auto itr = m_mailboxes.find(id.m_mailbox_id);
	return itr == end(m_mailboxes) ? nullptr : &itr->second;
}

Mbox<std::shared_ptr<Msg>> *Router::validate(const Net_ID &id)
{
	std::lock_guard<std::mutex> l(m_mutex);
	return validate_no_lock(id);
}

int32_t Router::poll(const std::vector<Net_ID> &ids)
{
	//given a list of net id's check to see if any of them contain mail.
	//return the index of the first one that does, else -1.
	std::lock_guard<std::mutex> l(m_mutex);
	std::vector<Mbox<std::shared_ptr<Msg>>*> mailboxes;
	mailboxes.reserve(ids.size());
	for (auto &id : ids) mailboxes.push_back(validate_no_lock(id));
	auto itr = std::find_if(begin(mailboxes), end(mailboxes),
		[&] (auto &mbox) { return !mbox->empty(); });
	if (itr == end(mailboxes)) return -1;
	return itr - begin(mailboxes);
}

int32_t Router::select(const std::vector<Net_ID> &ids)
{
	//block until one of the list of mailboxes contains some mail.
	//returns the index of the first mailbox that has mail.
	std::vector<Mbox<std::shared_ptr<Msg>>*> mailboxes;
	mailboxes.reserve(ids.size());
	{
		std::lock_guard<std::mutex> l(m_mutex);
		for (auto &id : ids) mailboxes.push_back(validate_no_lock(id));
	}
	auto itr = std::find_if(begin(mailboxes), end(mailboxes),
		[&] (auto &mbox) { return !mbox->empty(); });
	if (itr != end(mailboxes)) return itr - begin(mailboxes);
	//no mailbox has mail
	Sync select;
	for (auto &mbox : mailboxes)
	{
		mbox->lock();
		mbox->set_select(&select);
	}
	for (auto &mbox : mailboxes) mbox->unlock();
	select.wait();
	for (auto &mbox : mailboxes)
	{
		mbox->lock();
		mbox->set_select(nullptr);
	}
	itr = std::find_if(begin(mailboxes), end(mailboxes),
		[&] (auto &mbox) { return !mbox->empty(); });
	for (auto &mbox : mailboxes) mbox->unlock();
	return itr - begin(mailboxes);
}

///////////////////////
// directory management
///////////////////////

void Router::run()
{
	//distributed directory process.
	//pings out the service directory for this device to the peers now and again.
	//purge the messages and directory to clean up any ques and entires from
	//devices and services that vanish unexpectedly.
	while (m_running)
	{
		//wake every so often or if made to do so
		m_wake_mbox.read(std::chrono::milliseconds(DIRECTORY_PING_RATE));
		if (!m_running) break;

		//flood service directory to the network
		auto body = std::make_shared<std::string>(sizeof(Kernel_Service::Event_directory), '\0');
		auto event_body = (Kernel_Service::Event_directory*)&*(body->begin());
		event_body->m_evt = Kernel_Service::evt_directory;
		event_body->m_src = global_router->alloc_src();
		event_body->m_via = global_router->get_node_id();
		event_body->m_hops = 0;
		{
			std::lock_guard<std::mutex> l(m_mutex);
			auto &dir_struct = m_directory[global_router->get_node_id()];
			for (auto &entry : dir_struct.m_services) body->append(entry).append("\n");
		}
		//broadcast to the list of known router peers
		for (auto &peer : global_router->get_peers())
		{
			auto msg = std::make_shared<Msg>(body);
			msg->set_dest(Net_ID(peer, Mailbox_ID{0}));
			global_router->send(msg);
		}

		//purge old external directory entires and routes
		purge_dir();
		purge_routes();
	}
}

void Router::stop_thread()
{
	if (!m_running) return;
	m_running = false;
	auto wake = this;
	m_wake_mbox.post(wake);
}

std::string Router::declare(const Net_ID &id, const std::string &service, const std::string &params)
{
	//declare a new local service.
	//create a new service entry in the directory.
	//wake the manager thread to make it flood out the new state.
	auto entry = service + "," + id.to_string() + "," + params;
	auto wake = this;
	std::lock_guard<std::mutex> l(m_mutex);
	m_directory[global_router->get_node_id()].m_services.insert(entry);
	m_wake_mbox.post(wake);
	return entry;
}

void Router::forget(const std::string &entry)
{
	//remove a local directory entry.
	//wake the manager thread to make it flood out the new state.
	auto wake = this;
	std::lock_guard<std::mutex> l(m_mutex);
	m_directory[global_router->get_node_id()].m_services.erase(entry);
	m_wake_mbox.post(wake);
}

bool Router::update_dir(const std::string &body)
{
	//update our service directory based on this ping message body
	auto event_body = (Kernel_Service::Event_directory*)&(*begin(body));
	auto event_body_end = &(*begin(body)) + body.size();
	std::lock_guard<std::mutex> l(m_mutex);
	auto &dir_struct = m_directory[event_body->m_src.m_node_id];
	//not a new session, so ignore !
	if (event_body->m_src.m_mailbox_id.m_id <= dir_struct.m_session) return false;
	dir_struct.m_session = event_body->m_src.m_mailbox_id.m_id;
	dir_struct.m_time_modified = std::chrono::high_resolution_clock::now();
	dir_struct.m_services.clear();
	//split the body into separate service entry strings.
	//insert them into the directory.
	for (auto &entry : split_string(std::string((const char*)event_body->m_data, event_body_end), "\n"))
	{
		dir_struct.m_services.insert(entry);
	}
	return true;
}

void Router::purge_dir()
{
	//remove any entries that are too old
	std::lock_guard<std::mutex> l(m_mutex);
	auto now = std::chrono::high_resolution_clock::now();
	auto itr = begin(m_directory);
	while (itr != end (m_directory))
	{
		if (global_router->get_node_id() != itr->first
			&& now - itr->second.m_time_modified >= std::chrono::milliseconds(MAX_DIRECTORY_AGE))
		{
			itr = m_directory.erase(itr);
		}
		else itr++;
	}
}

std::vector<std::string> Router::enquire(const std::string &prefix)
{
	//return vector of all service entires with this prefix
	auto services = std::vector<std::string>{};
	std::lock_guard<std::mutex> l(m_mutex);
	for (auto &entry : m_directory)
	{
		auto &set = entry.second.m_services;
		std::copy_if(cbegin(set), cend(set), std::inserter(services, services.end()), [&] (auto &&s)
		{
			return strncmp(s.c_str(), prefix.c_str(), prefix.size()) == 0;
		});
	}
	return services;
}

std::vector<std::string> Router::enquire(const Node_ID &id, const std::string &prefix)
{
	//return vector of all service entires for given device with this prefix
	auto services = std::vector<std::string>{};
	std::lock_guard<std::mutex> l(m_mutex);
	auto &set = m_directory[id].m_services;
	std::copy_if(cbegin(set), cend(set), std::inserter(services, services.end()), [&] (auto &&s)
	{
		return strncmp(s.c_str(), prefix.c_str(), prefix.size()) == 0;
	});
	return services;
}

/////////
// router
/////////

void Router::send(std::shared_ptr<Msg> &msg)
{
	//this one method is responsible for all message sending and receiving !
	//it does all the fragmentation, reconstruction and forwarding required.
	std::lock_guard<std::mutex> l(m_mutex);
	//is message for this device ?
	if (msg->m_header.m_dest.m_node_id == m_node_id)
	{
		//yes so validate the mbox id
		auto mbox = validate_no_lock(msg->m_header.m_dest.m_mailbox_id);
		if (!mbox) return;
		//is this only a fragment of a parcel ?
		if (msg->m_header.m_total_length > msg->m_header.m_frag_length)
		{
			//look up parcel, create if not found.
			auto &parcel = m_parcels[msg->m_header.m_src];
			if (!parcel.m_msg)
			{
				//timestamp for purging
				auto now = std::chrono::high_resolution_clock::now();
				parcel = Que_Item{now, std::make_shared<Msg>(msg->m_header.m_total_length, msg->m_header.m_total_length)};
			}
			//fill in this slice, do we now have it all ?
			memcpy(parcel.m_msg->begin() + msg->m_header.m_frag_offset, msg->begin(), msg->m_header.m_frag_length);
			parcel.m_msg->m_header.m_total_length -= msg->m_header.m_frag_length;
			if (parcel.m_msg->m_header.m_total_length) return;
			//got it all now, so remove it and post the full message
			auto src = msg->m_header.m_src;
			auto dst = msg->m_header.m_dest;
			msg = std::move(parcel.m_msg);
			msg->set_dest(dst);
			m_parcels.erase(src);
		}
		//post msg to mailbox
		mbox->post(msg);
	}
	else
	{
		//going off device, so do we need to break it down to the max packet size ?
		auto now = std::chrono::high_resolution_clock::now();
		if (msg->m_header.m_frag_length > lk_data_size)
		{
			//yes, ok que msgs that reference slices of the data buffer
			auto src = alloc_src_no_lock();
			auto total_size = (uint32_t)msg->m_data->size();
			for (auto frag_offset = 0u; frag_offset < msg->m_header.m_frag_length;)
			{
				auto frag_size = std::min(lk_data_size, msg->m_header.m_frag_length - frag_offset);
				m_outgoing_msg_que.emplace_back(Que_Item{now, std::make_shared<Msg>(msg->m_data, msg->m_header.m_dest, src, total_size, frag_size, frag_offset)});
				frag_offset += frag_size;
			}
		}
		else
		{
			//que on the outgoing messages que
			m_outgoing_msg_que.emplace_back(Que_Item{now, std::move(msg)});
		}
		//wake the links to get them sending
		m_cv.notify_all();
	}
}

std::shared_ptr<Msg> Router::read(const Net_ID &id)
{
	auto mbox = global_router->validate(id);
	return mbox->read();
}

Net_ID Router::alloc_src_no_lock()
{
	auto src = Net_ID(m_node_id, m_next_parcel_id);
	m_next_parcel_id.m_id++;
	return src;
}

Net_ID Router::alloc_src()
{
	std::lock_guard<std::mutex> l(m_mutex);
	return alloc_src_no_lock();
}

bool Router::update_route(const std::string &body)
{
	//update our routing table based on this ping message body
	auto event_body = (Kernel_Service::Event_directory*)&(*begin(body));
	std::lock_guard<std::mutex> l(m_mutex);
	auto &route_struct = m_routes[event_body->m_src.m_node_id];
	//ignore if it's from an old session !
	if (event_body->m_src.m_mailbox_id.m_id < route_struct.m_session) return false;
	if (event_body->m_src.m_mailbox_id.m_id != route_struct.m_session)
	{
		//new session, so purge and create a new entry
		route_struct.m_session = event_body->m_src.m_mailbox_id.m_id;
		route_struct.m_time = std::chrono::high_resolution_clock::now();
		route_struct.m_hops = event_body->m_hops;
		route_struct.m_vias.clear();
		route_struct.m_vias.insert(event_body->m_via);
	}
	else if (event_body->m_hops < route_struct.m_hops)
	{
		//much better route, purge any existing routes and replace with this one
		route_struct.m_hops = event_body->m_hops;
		route_struct.m_vias.clear();
		route_struct.m_vias.insert(event_body->m_via);
	}
	else if (event_body->m_hops == route_struct.m_hops)
	{
		//equally good route, so add this as a viable route
		route_struct.m_vias.insert(event_body->m_via);
	}
	return true;
}

void Router::purge_routes()
{
	//purge mail ques and routing tables of any old entries
	std::lock_guard<std::mutex> l(m_mutex);
	auto now = std::chrono::high_resolution_clock::now();

	//purge routing tables
	for (auto itr = begin(m_routes); itr != end (m_routes);)
	{
		if (m_node_id != itr->first
			&& now - itr->second.m_time >= std::chrono::milliseconds(MAX_ROUTE_AGE))
		{
			itr = m_routes.erase(itr);
		}
		else itr++;
	}

	//purge stale messages and parcels
	for (auto itr = begin(m_outgoing_msg_que); itr != end (m_outgoing_msg_que);)
	{
		if (now - itr->m_time >= std::chrono::milliseconds(MAX_MESSAGE_AGE))
		{
			itr = m_outgoing_msg_que.erase(itr);
		}
		else itr++;
	}
	for (auto itr = begin(m_parcels); itr != end (m_parcels);)
	{
		if (now - itr->second.m_time >= std::chrono::milliseconds(MAX_PARCEL_AGE))
		{
			itr = m_parcels.erase(itr);
		}
		else itr++;
	}
}

std::shared_ptr<Msg> Router::get_next_msg(const Node_ID &dest, std::chrono::milliseconds timeout)
{
	//get next message bound for the destination device else nullptr
	auto poll_que = [&]() -> std::shared_ptr<Msg>
	{
		auto itr = std::find_if(begin(m_outgoing_msg_que), end(m_outgoing_msg_que), [&] (const auto &qi)
		{
			if (dest == qi.m_msg->m_header.m_dest.m_node_id) return true;
			auto &route_struct = m_routes[qi.m_msg->m_header.m_dest.m_node_id];
			return route_struct.m_vias.find(dest) != end(route_struct.m_vias);
		});
		if (itr == end(m_outgoing_msg_que)) return nullptr;
		auto msg = std::move((*itr).m_msg);
		m_outgoing_msg_que.erase(itr);
		return msg;
	};
	//if there is no viable message for this destination then block till somthing
	//new turns up on the que
	std::unique_lock<std::mutex> l(m_mutex);
	auto msg = poll_que();
	if (!msg) m_cv.wait_for(l, timeout, [&]{ return (msg = poll_que()) != nullptr; });
	return msg;
}

void Router::add_link(Link *link, const Node_ID &id)
{
	//new link driver entry that can send to given peer
	std::lock_guard<std::mutex> l(m_mutex);
	m_links[link] = id;
}

void Router::sub_link(Link *link)
{
	//remove link driver entry
	std::lock_guard<std::mutex> l(m_mutex);
	auto itr = m_links.find(link);
	if (itr != end(m_links)) m_links.erase(itr);
}

std::vector<Node_ID> Router::get_peers()
{
	//get a list of all current peer devices
	auto peers = std::vector<Node_ID>{};
	std::lock_guard<std::mutex> l(m_mutex);
	for (auto &link : m_links)
	{
		if (std::find(begin(peers), end(peers), link.second) == end(peers))
		{
			peers.push_back(link.second);
		}
	}
	return peers;
}

void Router::broadcast(const std::vector<std::string> &services, std::shared_ptr<std::string> &body, const Net_ID &id)
{
	//utility to broadcast a message body to a given list of services.
	//optionally ignore a given service, for example yourself.
	for (auto &entry : services)
	{
		auto fields = split_string(entry, ",");
		auto service_id = Net_ID::from_string(fields[1]);
		if (service_id == id) continue;
		auto msg = std::make_shared<Msg>(body);
		msg->set_dest(service_id);
		send(msg);
	}
}
