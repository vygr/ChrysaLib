#include "../../lib/services/kernel_service.h"
#include "../../lib/links/ip_link.h"
#include "../../lib/links/usb_link.h"
#include <iostream>
#include <sstream>

//////
// hub
//////

void ss_reset(std::stringstream &ss, std::string s)
{
	ss.str(s);
	ss.clear();
}

int32_t main(int32_t argc, char *argv[])
{
	//process comand args
	std::string arg_ip;
	std::string arg_usb;
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
			else if (opt == "t")
			{
				if (++i >= argc) goto help;
				ss_reset(ss, argv[i]);
				ss >> arg_t;
			}
			else
			{
			help:
				std::cout << "hub_node [switches] ip_addr ...\n";
				std::cout << "eg. hub_node -t 10000 -usb -ip 192.168.0.64 192.168.0.65\n";
				std::cout << "-h:    this help info\n";
				std::cout << "-t ms: exit timeout, default 0, ie never\n";
				std::cout << "-usb:  start the usb link manager\n";
				std::cout << "-ip:   start the ip link manager server\n";
				exit(0);
			}
		}
		else
		{
			//arg
			arg_dial.push_back(argv[i]);
		}
	}

	//vars
	std::unique_ptr<Router> m_router;
	std::unique_ptr<Kernel_Service> m_kernel;
	std::unique_ptr<USB_Link_Manager> m_usb_link_manager;
	std::unique_ptr<IP_Link_Manager> m_ip_link_manager;

	//startup, kernel is first service so it gets Mailbox_ID 0
	m_router = std::make_unique<Router>();
	m_kernel = std::make_unique<Kernel_Service>(*m_router);
	m_kernel->start_thread();
	if (arg_usb != "")
	{
		std::cout << "Starting USB link manager" << std::endl;
		m_usb_link_manager = std::make_unique<USB_Link_Manager>(*m_router);
		m_usb_link_manager->start_thread();
	}
	if (arg_ip != "" || !arg_dial.empty())
	{
		std::cout << "Starting IP link manager" << std::endl;
		m_ip_link_manager = std::make_unique<IP_Link_Manager>(*m_router, arg_ip);
		m_ip_link_manager->start_thread();
		for (auto &addr : arg_dial)
		{
			std::cout << "Dialing " << addr << std::endl;
			m_ip_link_manager->dial(addr);
		}
	}

	//print any changes to service directory
	auto start = std::chrono::high_resolution_clock::now();
	auto old_entries = std::vector<std::string>{};
	for (;;)
	{
		auto entries = m_router->enquire("");
		if (entries != old_entries)
		{
			std::cout << "+-----------+" << std::endl;
			std::cout << "| Directory |" << std::endl;
			std::cout << "+-----------+" << std::endl;
			old_entries = entries;
			for (auto &e : entries)
			{
				auto fields = split_string(e, ",");
				std::cout << "Service: " << fields[0] << " Info: " << fields[2] << std::endl;
				std::cout << "Net_ID: " << fields[1] << std::endl;
			}
		}

		auto finish = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed = finish - start;
		if (arg_t != 0 && elapsed.count() > arg_t) break;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	//shutdown
	if (m_usb_link_manager) m_usb_link_manager->stop_thread();
	if (m_ip_link_manager) m_ip_link_manager->stop_thread();
	m_kernel->stop_thread();
	if (m_usb_link_manager) m_usb_link_manager->join_thread();
	if (m_ip_link_manager) m_ip_link_manager->join_thread();
	m_kernel->join_thread();

	return 0;
}
