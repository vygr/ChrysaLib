#include "../../lib/services/kernel_service.h"
#include "../../lib/links/ip_link.h"
#include "../../lib/links/usb_link.h"
#include "../../lib/links/shm_link.h"
#include <iostream>
#include <sstream>

//////
// hub
//////

std::unique_ptr<Router> global_router;
std::thread::id global_kernel_thread_id;
uint32_t arg_v = 0;

void ss_reset(std::stringstream &ss, std::string s)
{
	ss.str(s);
	ss.clear();
}

int32_t main(int32_t argc, char *argv[])
{
	//process command args
	std::string arg_ip;
	std::string arg_usb;
	std::string arg_shm;
	auto arg_t = 0U;
	std::vector<std::string> arg_dial;
	std::stringstream ss;
	for (auto i = 1; i < argc; ++i)
	{
		if (argv[i][0] == '-')
		{
			//switch
			std::string opt = argv[i];
			while (!opt.empty() && opt[0] == '-') opt.erase(0, 1);
			if (opt == "ip") arg_ip = "server";
			else if (opt == "usb") arg_usb = "on";
			else if (opt == "shm") arg_shm = "on";
			else if (opt == "t")
			{
				if (++i >= argc) goto help;
				ss_reset(ss, argv[i]);
				ss >> arg_t;
			}
			else if (opt == "v")
			{
				if (++i >= argc) goto help;
				ss_reset(ss, argv[i]);
				ss >> arg_v;
			}
			else
			{
			help:
				std::cout << "hub_node [switches] [ip_addr ...]\n";
				std::cout << "eg. hub_node -t 10000 -usb -ip 192.168.0.64 192.168.0.65\n";
				std::cout << "-h:       this help info\n";
				std::cout << "-v level: verbosity, default 0, ie none\n";
				std::cout << "-t ms:    exit timeout, default 0, ie never\n";
				std::cout << "-usb:     start the usb link manager\n";
				std::cout << "-ip:      start the ip link manager server\n";
				std::cout << "-shm:     start the shmem link manager server\n";
				exit(0);
			}
		}
		else
		{
			//arg
			arg_dial.push_back(argv[i]);
		}
	}

	//globals
	global_kernel_thread_id = std::this_thread::get_id();
	global_router = std::make_unique<Router>();

	//vars
	std::shared_ptr<Kernel_Service> m_kernel;
	std::unique_ptr<USB_Link_Manager> m_usb_link_manager;
	std::unique_ptr<IP_Link_Manager> m_ip_link_manager;
	std::unique_ptr<SHM_Link_Manager> m_shm_link_manager;

	//startup, kernel is first service so it gets Mailbox_ID 0
	std::cout << std::endl;
	std::cout << "+----------+" << std::endl;
	std::cout << "| Hub node |" << std::endl;
	std::cout << "+----------+" << std::endl;
	m_kernel = std::make_shared<Kernel_Service>();
	m_kernel->start_thread();
	if (arg_usb != "")
	{
		if (arg_v > 1) std::cout << "Starting USB link manager" << std::endl;
		m_usb_link_manager = std::make_unique<USB_Link_Manager>();
		m_usb_link_manager->start_thread();
	}
	if (arg_shm != "")
	{
		if (arg_v > 1) std::cout << "Starting SHMem link manager" << std::endl;
		m_shm_link_manager = std::make_unique<SHM_Link_Manager>();
		m_shm_link_manager->start_thread();
	}
	if (arg_ip != "" || !arg_dial.empty())
	{
		if (arg_v > 1) std::cout << "Starting IP link manager" << std::endl;
		m_ip_link_manager = std::make_unique<IP_Link_Manager>(arg_ip);
		m_ip_link_manager->start_thread();
		for (auto &addr : arg_dial)
		{
			if (arg_v > 1) std::cout << "Dialing " << addr << std::endl;
			m_ip_link_manager->dial(addr);
		}
	}

	//print any changes to service directory
	auto start = std::chrono::high_resolution_clock::now();
	auto old_entries = std::vector<std::string>{};
	for (;;)
	{
		auto entries = global_router->enquire("");
		if (entries != old_entries)
		{
			old_entries = entries;
			if (arg_v > 1)
			{
				auto items = entries.size();
				auto label = (items == 1) ? " Item |" : " Items |";
				auto padding = std::string(std::to_string(items).length()+strlen(label), '-');
				auto topntail = "+-----------" + padding + "+";
				std::cout << topntail << std::endl;
				std::cout << "| Directory: " << items << label << std::endl;
				std::cout << topntail << std::endl;

				for (auto &e : entries)
				{
					auto fields = split_string(e, ",");
					std::cout << "Service: '" << fields[0];
					std::cout << "', Info: '" << fields[2];
					std::cout << "', Net_ID: '" << fields[1];
					std::cout << "'" << std::endl;
				}
			}
		}

		auto finish = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed = finish - start;
		if (arg_t != 0 && elapsed.count() > arg_t) break;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	//shutdown
	if (m_shm_link_manager) m_shm_link_manager->stop_thread();
	if (m_usb_link_manager) m_usb_link_manager->stop_thread();
	if (m_ip_link_manager) m_ip_link_manager->stop_thread();
	m_kernel->stop_thread();
	if (m_shm_link_manager) m_shm_link_manager->join_thread();
	if (m_usb_link_manager) m_usb_link_manager->join_thread();
	if (m_ip_link_manager) m_ip_link_manager->join_thread();
	m_kernel->join_thread();

	return 0;
}
