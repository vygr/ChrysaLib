#include "../../lib/services/kernel_service.h"
#include "../../lib/services/gui_service.h"
#include "../../lib/links/ip_link.h"
#include <iostream>
#include <sstream>

//////////
// gui app
//////////

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
			if (opt == "t")
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
				std::cout << "gui_node [switches] [ip_addr ...]\n";
				std::cout << "eg. gui_node -t 10000 127.0.0.1\n";
				std::cout << "-h:       this help info\n";
				std::cout << "-v level: verbosity, default 0, ie none\n";
				std::cout << "-t ms:    exit timeout, default 0, ie never\n";
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
	std::unique_ptr<IP_Link_Manager> m_ip_link_manager;
	std::shared_ptr<GUI_Service> m_gui;

	//startup, kernel is first service so it gets Mailbox_ID 0
	m_kernel = std::make_shared<Kernel_Service>();
	m_gui = std::make_shared<GUI_Service>();
	m_gui->start_thread();
	if (!arg_dial.empty())
	{
		if (arg_v > 1) std::cout << "Starting IP link manager" << std::endl;
		m_ip_link_manager = std::make_unique<IP_Link_Manager>("");
		m_ip_link_manager->start_thread();
		for (auto &addr : arg_dial)
		{
			if (arg_v > 1) std::cout << "Dialing " << addr << std::endl;
			m_ip_link_manager->dial(addr);
		}
	}

	//jump into Kernel run method !!!
	//it has to be the main thread
	//it's just forced on us by so many libs !
	m_kernel->m_running = true;
	m_kernel->run();

	//shutdown
	if (m_ip_link_manager) m_ip_link_manager->stop_thread();
	m_gui->stop_thread();
	if (m_ip_link_manager) m_ip_link_manager->join_thread();
	m_gui->join_thread();

	return 0;
}
