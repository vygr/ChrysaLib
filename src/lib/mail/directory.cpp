#include "directory.h"
#include "../settings.h"
#include "../services/kernel_service.h"
#include <algorithm>
#include <cstring>

///////////////////////
// directory management
///////////////////////

void Directory_Manager::run()
{
	//distributed directory process.
	//pings out the service directory for this device to the peers now and again.
	//purge the messages and directory to clean up any ques and entires from
	//devices and services that vanish unexpectedly.
	m_running = true;
	while (m_running)
	{
		//wake every so often or if made to do so
		m_wake_mbox.read(std::chrono::milliseconds(DIRECTORY_PING_RATE));
		if (!m_running) break;

		//flood service directory to the network
		auto body = std::make_shared<std::string>(sizeof(Kernel_Service::Event_directory), '\0');
		auto body_struct = (Kernel_Service::Event_directory*)&*(body->begin());
		body_struct->m_evt = Kernel_Service::evt_directory;
		body_struct->m_src = m_router.alloc_src();
		body_struct->m_via = m_router.get_dev_id();
		body_struct->m_hops = 0;
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			auto &dir_struct = m_directory[m_router.get_dev_id()];
			for (auto &entry : dir_struct.m_services) body->append(entry).append("\n");
		}
		//broadcast to the list of known router peers
		for (auto &peer : m_router.get_peers())
		{
			auto msg = std::make_shared<Msg>(body);
			msg->set_dest(Net_ID(peer, Mailbox_ID{0}));
			m_router.send(msg);
		}

		//purge old external directory entires and routes
		purge();
		m_router.purge();
	}
}

void Directory_Manager::stop_thread()
{
	if (!m_running) return;
	m_running = false;
	auto wake = this;
	m_wake_mbox.post(wake);
}

std::string Directory_Manager::declare(const Net_ID &id, const std::string &service, const std::string &params)
{
	//declare a new local service.
	//create a new service entry in the directory.
	//wake the manager thread to make it flood out the new state.
	auto entry = service + "," + id.to_string() + "," + params;
	auto wake = this;
	std::lock_guard<std::mutex> lock(m_mutex);
	m_directory[m_router.get_dev_id()].m_services.insert(entry);
	m_wake_mbox.post(wake);
	return entry;
}

void Directory_Manager::forget(const std::string &entry)
{
	//remove a local directory entry.
	//wake the manager thread to make it flood out the new state.
	auto wake = this;
	std::lock_guard<std::mutex> lock(m_mutex);
	m_directory[m_router.get_dev_id()].m_services.erase(entry);
	m_wake_mbox.post(wake);
}

bool Directory_Manager::update(const std::string &body)
{
	//update our service directory based on this ping message body
	auto event = (Kernel_Service::Event_directory*)&(*begin(body));
	auto body_end = &(*begin(body)) + body.size();
	std::lock_guard<std::mutex> lock(m_mutex);
	auto &dir_struct = m_directory[event->m_src.m_device_id];
	//not a new session, so ignore !
	if (event->m_src.m_mailbox_id.m_id <= dir_struct.m_session) return false;
	dir_struct.m_session = event->m_src.m_mailbox_id.m_id;
	dir_struct.m_time_modified = std::chrono::high_resolution_clock::now();
	dir_struct.m_services.clear();
	//split the body into separate service entry strings.
	//insert them into the directory.
	for (auto &entry : split_string(std::string((const char*)event->m_data, body_end), "\n"))
	{
		dir_struct.m_services.insert(entry);
	}
	return true;
}

void Directory_Manager::purge()
{
	//remove any entries that are too old
	std::lock_guard<std::mutex> lock(m_mutex);
	auto now = std::chrono::high_resolution_clock::now();
	auto itr = begin(m_directory);
	while (itr != end (m_directory))
	{
		if (m_router.get_dev_id() != itr->first
			&& now - itr->second.m_time_modified >= std::chrono::milliseconds(MAX_DIRECTORY_AGE))
		{
			itr = m_directory.erase(itr);
		}
		else itr++;
	}
}

std::vector<std::string> Directory_Manager::enquire(const std::string &prefix)
{
	//return vector of all service entires with this prefix
	auto services = std::vector<std::string>{};
	std::lock_guard<std::mutex> lock(m_mutex);
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

std::vector<std::string> Directory_Manager::enquire(const Dev_ID &dev_id, const std::string &prefix)
{
	//return vector of all service entires for given device with this prefix
	auto services = std::vector<std::string>{};
	std::lock_guard<std::mutex> lock(m_mutex);
	auto &set = m_directory[dev_id].m_services;
	std::copy_if(cbegin(set), cend(set), std::inserter(services, services.end()), [&] (auto &&s)
	{
		return strncmp(s.c_str(), prefix.c_str(), prefix.size()) == 0;
	});
	return services;
}
