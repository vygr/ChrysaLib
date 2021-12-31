#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "net.h"
#include <set>

//directory management.
//maintain a set of services for each node, on each node.
//a service entry is of the format: "service_name,dev_id:mbox_id,..."

class Router;

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

class Directory_Manager
{
public:
	Directory_Manager(Router &router)
		: m_router(router)
	{}
	//thread runs this run method
	void run();
	void stop_thread();
	//nameing of mailboxes
	std::string declare(const Net_ID &id, const std::string &service, const std::string &params);
	void forget(const std::string &entry);
	std::vector<std::string> enquire(const std::string &prefix);
	std::vector<std::string> enquire(const Dev_ID &dev_id, const std::string &prefix);
	//directory updating
	bool update(const std::string &body);
	bool m_running = false;
private:
	void purge();
	std::mutex m_mutex;
	Router &m_router;
	std::map<Dev_ID, Directory> m_directory;
	Mbox<Directory_Manager*> m_wake_mbox;
};

#endif
