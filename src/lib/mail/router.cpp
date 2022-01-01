#include "router.h"
#include "../services/kernel_service.h"
#include <algorithm>
#include <cstring>

/////////
// router
/////////

void Router::send(std::shared_ptr<Msg> &msg)
{
	//this one method is responsible for all message sending and receiving !
	//it does all the fragmentation, reconstruction and forwarding required.
	std::lock_guard<std::mutex> lock(m_mutex);
	//is message for this device ?
	if (msg->m_header.m_dest.m_device_id == m_device_id)
	{
		//yes so validate the mbox id
		auto mbox = m_mailbox_manager.validate(msg->m_header.m_dest.m_mailbox_id);
		if (!mbox) return;
		//is this only a fragment of parcel ?
		if (msg->m_header.m_frag_length < msg->m_header.m_total_length)
		{
			//look up parcel, create if not found, parcel.first will be 0 if just created.
			auto &parcel = m_parcels[msg->m_header.m_src];
			if (!parcel.second.m_msg)
			{
				//timestamp for purgeing
				auto now = std::chrono::high_resolution_clock::now();
				parcel.second = Que_Item{now, std::make_shared<Msg>(msg->m_header.m_total_length)};
			}
			//fill in this slice, do we now have it all ?
			memcpy(parcel.second.m_msg->begin() + msg->m_header.m_frag_offset, msg->begin(), msg->m_header.m_frag_length);
			parcel.first += msg->m_header.m_frag_length;
			if (parcel.first != msg->m_header.m_total_length) return;
			//got it all now, so remove it and post the full message
			auto src = msg->m_header.m_src;
			auto dst = msg->m_header.m_dest;
			msg = std::move(parcel.second.m_msg);
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
		if (msg->m_header.m_frag_length > MAX_PACKET_SIZE)
		{
			//yes, ok que msgs that reference slices of the data buffer
			auto src = _alloc_src();
			for (auto frag_offset = 0u; frag_offset < msg->m_header.m_frag_length;)
			{
				auto frag_size = std::min(MAX_PACKET_SIZE, msg->m_header.m_frag_length - frag_offset);
				m_outgoing_msg_que.emplace_back(Que_Item{now, std::make_shared<Msg>(msg->m_data, msg->m_header.m_dest, src, frag_size, frag_offset)});
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

bool Router::update_route(const std::string &body)
{
	//update our routing table based on this ping message body
	auto event_body = (Kernel_Service::Event_directory*)&(*begin(body));
	std::lock_guard<std::mutex> lock(m_mutex);
	auto &route_struct = m_routes[event_body->m_src.m_device_id];
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

void Router::purge()
{
	//purge mail ques and routing tables of any old entries
	std::lock_guard<std::mutex> lock(m_mutex);
	auto now = std::chrono::high_resolution_clock::now();

	//purge routing tables
	for (auto itr = begin(m_routes); itr != end (m_routes);)
	{
		if (m_device_id != itr->first
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
		if (now - itr->second.second.m_time >= std::chrono::milliseconds(MAX_PARCEL_AGE))
		{
			itr = m_parcels.erase(itr);
		}
		else itr++;
	}
}

std::shared_ptr<Msg> Router::get_next_msg(const Dev_ID &dest, std::chrono::milliseconds timeout)
{
	//get next message bound for the destination device else nullptr
	auto poll_que = [&]() -> std::shared_ptr<Msg>
	{
		auto itr = std::find_if(begin(m_outgoing_msg_que), end(m_outgoing_msg_que), [&] (const auto &qi)
		{
			if (dest == qi.m_msg->m_header.m_dest.m_device_id) return true;
			auto &route_struct = m_routes[qi.m_msg->m_header.m_dest.m_device_id];
			return route_struct.m_vias.find(dest) != end(route_struct.m_vias);
		});
		if (itr == end(m_outgoing_msg_que)) return nullptr;
		auto msg = std::move((*itr).m_msg);
		m_outgoing_msg_que.erase(itr);
		return msg;
	};
	//if there is no viable message for this destination then block till somthing
	//new turns up on the que
	std::unique_lock<std::mutex> lock(m_mutex);
	auto msg = poll_que();
	if (!msg) m_cv.wait_for(lock, timeout, [&]{ return (msg = poll_que()) != nullptr; });
	return msg;
}

void Router::add_link(Link *link, const Dev_ID &id)
{
	//new link driver entry that can send to given peer
	std::lock_guard<std::mutex> lock(m_mutex);
	m_links[link] = id;
}

void Router::sub_link(Link *link)
{
	//remove link driver entry
	std::lock_guard<std::mutex> lock(m_mutex);
	auto itr = m_links.find(link);
	if (itr != end(m_links)) m_links.erase(itr);
}

std::vector<Dev_ID> Router::get_peers()
{
	//get a list of all current peer devices
	auto peers = std::vector<Dev_ID>{};
	std::lock_guard<std::mutex> lock(m_mutex);
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
